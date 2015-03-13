/*
 * An instruction based stride prefetcher with state information that stores
 * the PC of load instructions together with the requested memory address and
 * calculates the stride when load instructions existing in the storage is 
 * encountered again. Prefetches are issued if the stride remains constant for
 * two consecutive load instructions that has got the same PC, where the
 * prefetch distance increases with each encounter of the load instruction
 * as long as the calculated stride remains constant.
 */

#include "interface.hh"
#include <map>
#include <deque>
#include <algorithm>

#define max_confidence 10
#define max_entries 242

using namespace std;


/* Data structures */
struct MyStat {
    Addr mem_addr;
    int64_t stride;
    int confidence;
};

deque<Addr> mydeque;
map<Addr, MyStat> mymap;

int64_t stride;
char confidence;
Addr remove_pc;
Addr pf_addr;
deque<Addr>::iterator it;

void prefetch_init(void)
{

    DPRINTF(HWPrefetch, "Initialized prefetcher\n");

}

void prefetch_access(AccessStat stat)
{

    stride = 0;
    confidence = 0;
    MyStat entry = {stat.mem_addr, stride, confidence};

    pair<map<Addr, MyStat>::iterator, bool> ret;
    ret = mymap.insert(pair<Addr, MyStat>(stat.pc, entry));
    if (!ret.second) {
        DPRINTF(HWPrefetch, "Entry found in map!\n");
        // calculate new stride
        stride = stat.mem_addr - mymap[stat.pc].mem_addr;

        // calculate confidence
        if (stride == mymap[stat.pc].stride) {
            // stride unchanged
            if (mymap[stat.pc].confidence < max_confidence) {
                confidence = mymap[stat.pc].confidence + 1;
            }
            else {
                confidence = mymap[stat.pc].confidence;
            }
        }

        // delete old reference from deque
        it = find(mydeque.begin(), mydeque.end(), stat.pc);
        if (it != mydeque.end()) {
            DPRINTF(HWPrefetch, "MOVING TO TOP OF DEQUE, pc= %d\n", *it);
            mydeque.erase(it);
        }
    }
    else {
        //DPRINTF(HWPrefetch, "SIZE OF DEQUE: %d, SIZE OF MAP: %d\n", mydeque.size(), mymap.size());
        // entry not found
        // remove LRU element if structures are full
        if (mydeque.size() >= max_entries) {

            remove_pc = mydeque.back();
            DPRINTF(HWPrefetch, "REMOVING FROM STRUCTURES, pc= %d\n", remove_pc);
            mymap.erase(remove_pc);
            mydeque.pop_back();
        }
    }

    // update entry values
    entry.mem_addr = stat.mem_addr;
    entry.stride = stride;
    entry.confidence = confidence;

    // insert into structures
    mymap[stat.pc] = entry;
    mydeque.push_front(stat.pc);

    DPRINTF(HWPrefetch, "UPDATING ENTRY: pc= %d, mem_addr= %d, stride= %d, confidence= %d\n", stat.pc, stat.mem_addr, stride, confidence);

    //Addr pf_addr = stat.mem_addr + stride; // + BLOCK_SIZE;

    /*
     * Issue a prefetch request if a demand miss occured,
     * and the block is not already in cache.
     */

    if (confidence > 0) {
        DPRINTF(HWPrefetch, "PREFETCH: mem_addr= %d, stride= %d, confidence= %d\n", stat.mem_addr, stride, confidence);
    }

    for (int i = 1; i <= confidence; i++) {
        pf_addr = stat.mem_addr + i*stride;
        if (pf_addr <= MAX_PHYS_MEM_ADDR && current_queue_size() < MAX_QUEUE_SIZE ) {
            issue_prefetch(pf_addr);
        }
    }
    //issue_prefetch(stat.mem_addr + 2*stride);

    //if (stat.miss && !in_cache(pf_addr)) {
    //    issue_prefetch(pf_addr);
    //}
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
