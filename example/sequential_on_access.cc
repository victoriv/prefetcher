/*
 * A sequential one-block lookahead prefetcher.
 * Ignores requests to blocks already in the cache.
 * Fetches next block on every access.
 *
 * author: Hallvard Norheim BÃ
 * NTNU, July 2010
 */

#include <stdio.h>
#include "interface.hh"


void prefetch_init(void)
{
    printf("Initialized sequential-on-access\n");
}

void prefetch_access(AccessStat stat)
{
    Addr pf_addr = stat.mem_addr + BLOCK_SIZE;

    if (!in_cache(pf_addr) && !in_mshr_queue(pf_addr))
        issue_prefetch(pf_addr);
}

void prefetch_complete(Addr addr) {}
