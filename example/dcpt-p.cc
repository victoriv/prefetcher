/*
 * DCPT-P (delta correlation prediction table - partial matching) prefetcher.
 *
 * Based on a paper by Grannaes, Jahre, Natvig:
 * "Multi-level Hardware Prefetching Using Low Complexity
 *  Delta Correlating Prediction Tables".
 *
 * This prefetcher does not seem to work well. Is there perhaps something
 * wrong with how partial matching is done?
 *
 * author: Hallvard Norheim Bø
 * NTNU, July 2010
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "interface.hh"
#include "base/trace.hh"


/* Header content */

#define TABLE_SIZE 100
#define DELTAS_PER_ENTRY 16
#define BITS_PER_DELTA 16
#define MASK_BITS 8

#define MAX_DELTA ((1 << ((BITS_PER_DELTA) - 1)) - 1)
#define MIN_DELTA ~(MAX_DELTA)

/* Delta_t must be at least BITS_PER_DELTA bits wide. */
typedef signed short Delta_t;

typedef struct {
    Addr pc;                            /* address of loading instruction */
    Addr last_address;                  /* previous load address */
    Addr last_prefetch;                 /* address of last prefetch */
    Delta_t deltas[DELTAS_PER_ENTRY];   /* delta buffer */
    int delta_pointer;                  /* index into the delta buffer */
} Entry;

static int table_index;
static Entry table[TABLE_SIZE];
static Addr candidates[DELTAS_PER_ENTRY - 2];

/* End of header content. */


/* Helper functions */

/* Search table for an entry matching pc */
static Entry *search_table(Addr pc)
{
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        if (table[i].pc == pc)
            return &table[i];
    }
    return NULL;

}

static Delta_t insert_delta(Entry *e, Delta_t d)
{
    e->deltas[e->delta_pointer] = d;
    e->delta_pointer = (e->delta_pointer + 1) % DELTAS_PER_ENTRY;
    return d;
}

/* indexes backwards from delta_pointer, offset=1 is last inserted delta */
static Delta_t get_delta(Entry *e, int offset)
{
    return e->deltas[(e->delta_pointer - offset + DELTAS_PER_ENTRY) % DELTAS_PER_ENTRY];
}

/* returns the index of the first pf candidate in candidates array */
int match_deltas(Entry *e, Delta_t bitmask)
{
    Delta_t d, p, delta_a, delta_b;
    int delta_a_found, delta_b_found;
    int i, j, k;
    int64_t delta;
    Addr pf_addr;

    /* start address for new pf candidates */
    pf_addr = e->last_address;

    delta_a_found = delta_b_found = 0;
    delta_a = get_delta(e, 1);
    delta_b = get_delta(e, 2);

    /* delta correlation: match two most recent deltas */
    for (i = 3, j = 0, k = 0, d = 0; i <= DELTAS_PER_ENTRY; i++) {
        p = d;
        d = get_delta(e, i);
        if (d == 0) {
            /* rest of buffer not initialized with deltas yet */
            break;
        }
        if (d == MIN_DELTA) {
            DPRINTF(HWPrefetch, "delta overflow, discarding all pf candidates\n");
            k = j;  /* Delta overflow, delete all prefetches */
            delta_a_found = delta_b_found = 0;
            continue;
        }
        if (delta_a_found && delta_b_found) {
            if ((bitmask & 1) == 0)
                DPRINTF(HWPrefetch, "found partial delta match: (%d, %d) == (%d, %d) using bitmask %#x\n", p, d, delta_a, delta_b, bitmask);

            /* generate prefetch candidate */
            delta = d * (BLOCK_SIZE >> 1);

            /* guards against pf_addr becoming "negative" and wrapping around */
            if (-delta <= (int64_t)pf_addr) {
                pf_addr += delta;
                assert(j < sizeof(candidates) / sizeof(candidates[0]));
                candidates[j++] = pf_addr;
                if (pf_addr == e->last_prefetch) {
                    DPRINTF(HWPrefetch, "matching last prefetch issued, discarding pf candidates\n");
                    k = j; /* discard all prefetch candidates */
                }
            }
            continue;
        }
        /* FIXME: add bitmasks here */
        if (delta_a_found && !delta_b_found) {
            if ((d & bitmask) == (delta_b & bitmask))
                delta_b_found = 1;
            else
                delta_a_found = 0;
        }
        if ((d & bitmask) == (delta_a & bitmask))
            delta_a_found = 1;
    }

    return k;
}


void prefetch_init(void)
{
    printf("Initialized dcpt-p,\nBITS_PER_DELTA=%d, MASK_BITS=%d\n", BITS_PER_DELTA, MASK_BITS);
}

void prefetch_access(AccessStat stat)
{
    int index;
    int64_t delta;
    Addr pf_addr;

    Entry *e = search_table(stat.pc);

    if (e != NULL) {

        /* calculate delta */
        delta = (int64_t)stat.mem_addr - (int64_t)e->last_address;
        delta /= BLOCK_SIZE >> 1; /* a delta of 2 covers a cache block */
        e->last_address = stat.mem_addr;

        if (delta != 0) {

            delta = MIN_DELTA < delta && delta <= MAX_DELTA ? delta : MIN_DELTA;

            /* insert new delta */
            insert_delta(e, (Delta_t)delta);

            /* clear prefetch candidate buffer */
            memset(candidates, 0, sizeof(candidates));

            /* try to find matching deltas (using all delta bits) */
            index = match_deltas(e, ~(Delta_t)0);

            /* match deltas with MASK_BITS lower bits masked out */
            if (candidates[index] == 0) {
                index = match_deltas(e, ~(Delta_t)(MASK_BITS - 1));
            }

            /* filter candidates and issue prefetches */
            for (; index < TABLE_SIZE && candidates[index]; index++) {
                pf_addr = candidates[index];
                if (!in_cache(pf_addr) && !in_mshr_queue(pf_addr) && current_queue_size() < MAX_QUEUE_SIZE) {
                    DPRINTF(HWPrefetch, "issuing pf request for %#llx\n", pf_addr);
                    issue_prefetch(pf_addr);
                    e->last_prefetch = pf_addr;
                } else {
                    DPRINTF(HWPrefetch, "discarded %#llx\n", pf_addr);
                }
            }
        }
    } else {
        /* Replace old entry in FIFO manner */
        e = &table[table_index];
        e->pc = stat.pc;
        e->last_address = stat.mem_addr;
        e->last_prefetch = 0;
        memset(e->deltas, 0, sizeof(e->deltas));
        e->delta_pointer = 0;
        table_index = (table_index + 1) % TABLE_SIZE;
    }
}

void prefetch_complete(Addr addr) {}
