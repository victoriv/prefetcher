/*
 * Tagged sequential prefetcher.
 * Newly prefetched blocks are tagged with a prefetch bit.
 * The prefetch bit is cleared on first reference.
 * When encountering a block with the prefetch bit set,
 * the next block is prefetched (and tagged).
 *
 * author: Hallvard Norheim Bø
 * NTNU, July 2010
 */

#include <stdio.h>
#include "interface.hh"

#define DEGREE 1


void prefetch_init(void)
{
    printf("Initialized tagged, DEGREE=%d\n", DEGREE);
}

void prefetch_access(AccessStat stat)
{
    int d;
    Addr addr = stat.mem_addr;

    if (stat.miss || get_prefetch_bit(addr)) {

        clear_prefetch_bit(addr);

        for (d = 1; d <= DEGREE; d++) {
            addr += BLOCK_SIZE;
            if (!in_cache(addr) && !in_mshr_queue(addr))
                issue_prefetch(addr);
        }
    }
}

void prefetch_complete(Addr addr) {
    set_prefetch_bit(addr);
}
