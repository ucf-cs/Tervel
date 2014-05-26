#include "tsigas_queue.h"
#include "node.h"

#include <assert.h>
#include <cds/container/tsigas_cycle_queue.h>
#include <cds/init.h>
#include <cds/gc/hp.h>
#include <limits.h>
#include <unistd.h>

#include <bitset>
#include <iostream>

void TsigasQueue::init(unsigned long num_threads, unsigned long capacity)
{ 
    // Initialize cds
    cds::Initialize() ;
    
    {
        // Initialize hazards pointer garbage collegtion
        cds::gc::HP hpGC;

        // attach a thread manager to the main thread using the container
        // but main thread shouldnt use the container and thus doesnt need
        // the next line
        //cds::threading::Manager::attachThread() ;
    }

    capacity_ = capacity;
    num_threads_ = num_threads;
    queue_ = new TQueue(capacity);
    
    if (capacity_ < 2 || (capacity_ & (capacity_-1))) {
        std::cerr << "Capacity was not a power of 2. This may result in errors or " 
                    << "inefficient use of space" << std::endl;
    }
}

void TsigasQueue::finish()
{
    cds::Terminate();
}

void TsigasQueue::attach_thread()
{
    // attach the thread to the cds thread manager
    cds::threading::Manager::attachThread();
}

void TsigasQueue::detach_thread()
{
    // attach the thread to the cds thread manager
    cds::threading::Manager::detachThread();
}

bool TsigasQueue::enqueue(Node *node)
{
    return queue_->enqueue(node);
}

bool TsigasQueue::dequeue()
{
    Node *val = NULL;
    return queue_->dequeue(val);
}
