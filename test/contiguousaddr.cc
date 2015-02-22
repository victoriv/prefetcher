/*
 * Sequential prefetcher that fetches the next eight contiguous
 * (non-block aligned) addresses.
 */
#include "interface.hh"

void prefetch_init(void) {}

void prefetch_access(AccessStat stat)
{
    Addr addr = stat.mem_addr;
    int i;

    for (i = 0; i < 8; i++)
        issue_prefetch(++addr);
}

void prefetch_complete(Addr addr) {}
