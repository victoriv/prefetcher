/*
 * Prefetcher that is quite likely to segfault.
 */
#include <stdio.h>
#include "interface.hh"

void prefetch_init(void) {}

void prefetch_access(AccessStat stat)
{
    if (stat.miss) {
        Addr address = stat.mem_addr + BLOCK_SIZE;
        printf("Issuing prefetch request to get %d!\n", *(int*)address);
        issue_prefetch(address);
    }
}

void prefetch_complete(Addr addr) {}
