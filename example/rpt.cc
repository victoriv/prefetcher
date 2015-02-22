/*
 * RPT (reference prediction table) prefetcher.
 *
 * Based on an earlier implemention from M5 version 1.1.
 * That version used a big switch statement for the state machine,
 * which has been replaced with a compact transition table.
 *
 * author: Hallvard Norheim Bø
 * NTNU, June 2010
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "interface.hh"
#include "base/trace.hh"

/* Header content */

#define TABLE_SIZE 64
#define DEGREE 1

typedef enum { INITIAL, TRANSIENT, STEADY, NONE } State;

State transition_table[2][4] = {
/*    INITIAL    TRANSIENT  STEADY   NONE */       /* current state */
    { TRANSIENT, NONE,      INITIAL, NONE      },  /* prediction was incorrect */
    { STEADY,    STEADY,    STEADY,  TRANSIENT },  /* prediction was correct */
};

typedef struct {
    Addr pc;        /* address of loading instruction */
    Addr prev;      /* previous memory address loaded by instruction */
    int stride;     /* detected stride */
    Tick time;      /* access time */
    State state;    /* current state */
} Entry;

static Entry table[TABLE_SIZE];

/* End of header content. */


void prefetch_init(void)
{
    printf("Initialized rpt,\nTABLE_SIZE=%d, DEGREE=%d\n", TABLE_SIZE, DEGREE);
}

void prefetch_access(AccessStat stat)
{
    int d, i, j;
    int correct;
    Addr blk_addr;
    Addr new_addr;
    Tick oldest;
    Entry *e = NULL;

    DPRINTF(HWPrefetch, "RPT received a prefetch requst [pc: %#llx, "\
            "addr: %#llx,  miss: %d, time: %lld\n", stat.pc, stat.mem_addr,
            stat.miss, stat.time);

    blk_addr = stat.mem_addr & ~(Addr)(BLOCK_SIZE-1);

    /* Search table for an entry matching the instruction address */
    for (i = 0; i < TABLE_SIZE; i++) {
        if (table[i].pc == stat.pc) {
            e = &table[i];
            DPRINTF(HWPrefetch, "found entry in table at index %d\n", i);
            break;
        }
    }

    if (e != NULL) {
        /* Found matching entry. */
        correct = (e->prev + e->stride == stat.mem_addr) ? 1 : 0;
        DPRINTF(HWPrefetch, "correct? %s\n", (correct ? "yes" : "no"));

        /* Update current entry. */
        if (e->state != STEADY && !correct)
            e->stride = stat.mem_addr - e->prev;
        e->state = transition_table[correct][e->state];
        e->prev = stat.mem_addr;
        e->time = stat.time;

        switch (e->state) {
            case INITIAL:   DPRINTF(HWPrefetch, "state: INITIAL\n"); break;
            case TRANSIENT: DPRINTF(HWPrefetch, "state: TRANSIENT\n"); break;
            case STEADY:    DPRINTF(HWPrefetch, "state: STEADY\n"); break;
            case NONE:      DPRINTF(HWPrefetch, "state: NONE\n"); break;
            default: assert(0); /* cannot happen */
        }

        if (e->state == STEADY && e->stride != 0) {
            /* Issue prefetches */
            for (d = 1; d <= DEGREE; d++) {
                assert(stat.mem_addr + d * e->stride > 0);
                new_addr = stat.mem_addr + d * e->stride;
                issue_prefetch(new_addr);
                DPRINTF(HWPrefetch, "issued prefetch for %#llx (stride %d)\n",
                        new_addr, e->stride);
            }
        }
    } else {
        /* Replace oldest entry */
        e = &table[0];
        oldest = e->time;
        for (j = 0, i = 1; i < TABLE_SIZE; i++) {
            if (table[i].time < oldest) {
                e = &table[i];
                oldest = e->time;
                j = i;
            }
        }
        DPRINTF(HWPrefetch, "replacing entry %d (state: INITIAL)\n", j);
        e->pc = stat.pc;
        e->prev = blk_addr;
        e->stride = 0;
        e->time = stat.time;
        e->state = INITIAL;
    }
}

void prefetch_complete(Addr addr) {}
