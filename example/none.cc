/*
 * Prefetcher skeleton using the prefetcher interface.
 */
#include "interface.hh"

class Prefetcher
{
  public:
    void prefetch_access(AccessStat stat);
};

void
Prefetcher::prefetch_access(AccessStat stat)
{
    return;
}


// Wrapper functions
static Prefetcher *prefetcher;
void prefetch_init(void) { prefetcher = new Prefetcher(); }
void prefetch_access(AccessStat stat) { prefetcher->prefetch_access(stat); }
void prefetch_complete(Addr addr) {}
