/*
 * Prefetcher with a forgotten print statement.
 */
#include <stdio.h>
#include "interface.hh"

void prefetch_init(void) {}


static int counter;

void prefetch_access(AccessStat stat)
{
    if (stat.miss) {
        Addr address = stat.mem_addr + BLOCK_SIZE;
        printf("Issuing prefetch request %d!\n", counter++);
        issue_prefetch(address);
    }
}

void prefetch_complete(Addr addr) {}
