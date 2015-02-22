/*
 * Sequential prefetcher that always fills the entire prefetch queue.
 */
#include "interface.hh"

void prefetch_init(void) {}

void prefetch_access(AccessStat stat)
{
    int d;

    /* Always fill up entire pf queue. */
    for (d = 1; d <= MAX_QUEUE_SIZE; d++)
        issue_prefetch(stat.mem_addr + d * BLOCK_SIZE);
}

void prefetch_complete(Addr addr) {}
