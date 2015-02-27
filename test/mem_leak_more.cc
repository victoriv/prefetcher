/*
 * Prefetcher with a large memory leak.
 */
#include <stdlib.h>
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "interface.hh"

void prefetch_init(void) {}

AccessStat* old_stat = NULL;

void prefetch_access(AccessStat stat)
{
    if (old_stat) {
        Addr address = stat.mem_addr + (stat.mem_addr - old_stat->mem_addr);
        if (address <= MAX_PHYS_MEM_ADDR)
            issue_prefetch(address);
        else
            printf("Skipped issue to %" PRIx64 "\n", address);
    }

    old_stat = (AccessStat*) malloc(sizeof(stat) * 1024);
    if (old_stat)
        *old_stat = stat;
    else
        fprintf(stderr, "Could not allocate! :(\n");
}

void prefetch_complete(Addr addr) {}
