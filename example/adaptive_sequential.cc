/*
 * Adaptive sequential prefetcher, based on a prefetching algorithm
 * described in Dahlgren, Dubois, Stenstrom (1995):
 * "Sequential Hardware Prefetching in Shared-Memory Multiprocessors".
 *
 * author: Hallvard Norheim Bø
 * NTNU, July 2010
 */

#include <stdio.h>
#include "interface.hh"

/* Macros used to get/set bits in the zero-bit array */
#define ZINDEX(addr) ((addr) / ((BLOCK_SIZE) * 8))
#define ZMASK(addr) (1 << ((addr) / (BLOCK_SIZE) & 3))

#define COUNTER_BITS 4
#define COUNTER_MAX (1 << COUNTER_BITS)
#define UPPER_THRESHOLD ((COUNTER_MAX * 12) / 16)
#define LOWER_THRESHOLD ((COUNTER_MAX * 8) / 16)
#define LOWEST_THRESHOLD ((COUNTER_MAX * 3) / 16)
#define START_PF_THRESHOLD ((COUNTER_MAX * 6) / 16)

/* The current degree of prefetching. */
unsigned short degree = 1;

/* The number of prefetches that have been returned after each read miss. */
unsigned short prefetches = 0;

/* The number of useful prefetches. */
unsigned short useful = 0;

/* zero-bit store */
/* FIXME: only needs bits for the cache, not entire memory! */
unsigned char zerobits[MAX_PHYS_MEM_ADDR / (BLOCK_SIZE * 8)];


/* get/set/clear zero bits */

int get_zero_bit(Addr addr)
{
    return zerobits[ZINDEX(addr)] & ZMASK(addr);
}

void set_zero_bit(Addr addr)
{
    zerobits[ZINDEX(addr)] |= ZMASK(addr);
}

void clear_zero_bit(Addr addr)
{
    zerobits[ZINDEX(addr)] &= ~ZMASK(addr);
}


void prefetch_init(void)
{
    printf("Initialized adaptive_sequential, COUNTER_BITS=%d\n", COUNTER_BITS);
}

void prefetch_access(AccessStat stat)
{
    int d;
    Addr addr, prev_addr;

    addr = stat.mem_addr & ~(Addr)(BLOCK_SIZE - 1);

    /* Book-keeping to check if prefetching should be turned back on */
    if (stat.miss && degree == 0) {
        set_zero_bit(addr);
        prev_addr = addr - BLOCK_SIZE;
        if (in_cache(prev_addr) && get_zero_bit(prev_addr)) {
            clear_zero_bit(prev_addr);
            useful++;
        }
    } else if (stat.miss && degree > 0)
        /* Clear off old zero flags */
        clear_zero_bit(addr);
    else {
        /* Count useful prefetches */
        if (get_prefetch_bit(addr)) {
            useful++;
            clear_prefetch_bit(addr);
        }
    }

    /* Check if degree of prefetching should be adjusted */
    if (prefetches == COUNTER_MAX) {
        if (degree == 0) {
            if (useful > START_PF_THRESHOLD)
                degree = 1;
        } else {
            if (useful > UPPER_THRESHOLD)
                degree++;
            else if (useful < LOWEST_THRESHOLD)
                degree >>= 1;
            else if(useful < LOWER_THRESHOLD)
                if (degree > 0)
                    degree--;
        }
        prefetches = 0;
        useful = 0;
    } else {
        prefetches++;
    }

    /* Issue prefetches */
    for (d = 1; d <= degree; d++) {
        addr += BLOCK_SIZE;
        if (!in_cache(addr))
            issue_prefetch(addr);
    }
}

void prefetch_complete(Addr addr) {
    set_prefetch_bit(addr);
}
