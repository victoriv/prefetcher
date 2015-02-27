/*
 * A sequential one-block lookahead prefetcher.
 * Ignores requests to blocks already in the cache.
 * Only fetches the next block if there is a cache miss.
 *
 * author: Hallvard Norheim BÃ
 * NTNU, July 2010
 */

#include <stdio.h>
#include "interface.hh"


void prefetch_init(void)
{
    printf("Initialized sequential-on-miss\n");
}

void prefetch_access(AccessStat stat)
{
    Addr pf_addr = stat.mem_addr + BLOCK_SIZE;

    if (stat.miss && !in_cache(pf_addr) && !in_mshr_queue(pf_addr))
        issue_prefetch(pf_addr);
}

void prefetch_complete(Addr addr) {}
