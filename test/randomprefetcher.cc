/*
 * Prefetches a random (legal) memory address.
 */

#include <stdlib.h>
#include <time.h>
#include "interface.hh"

void prefetch_init(void) {}

void prefetch_access(AccessStat stat)
{
    Addr rand_addr;
    srand(time(NULL));
    rand_addr = (rand() & (MAX_PHYS_MEM_ADDR - 1) / BLOCK_SIZE) * BLOCK_SIZE;
    issue_prefetch(rand_addr);
}

void prefetch_complete(Addr addr) {}
