/*
 * Single-minded prefetcher that always fetches the current block.
 */
#include "interface.hh"

void prefetch_init(void) {}

void prefetch_access(AccessStat stat)
{
    issue_prefetch(stat.mem_addr);
}

void prefetch_complete(Addr addr) {}
