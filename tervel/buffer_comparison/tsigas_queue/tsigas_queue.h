#ifndef __TSIGAS_BUFFER_H__
#define __TSIGAS_BUFFER_H__

#include "node.h"

#include <cds/container/tsigas_cycle_queue.h>
#include <limits.h>
#include <pthread.h>

#include <atomic>

const long kNotValue = LONG_MAX;

class TsigasQueue
{
private:

    typedef cds::container::TsigasCycleQueue<Node*,
                                     cds::opt::buffer< cds::opt::v::dynamic_buffer< Node* > > 
                                     > TQueue;
    TQueue *queue_;

    unsigned int num_threads_;
    unsigned long capacity_;

public:
    void init(unsigned long num_threads, unsigned long capacity);
    void finish();
    void attach_thread();
    void detach_thread();
    bool enqueue(Node *node);
    bool dequeue();
};

#endif  // __TSIGAS_BUFFER_H__
