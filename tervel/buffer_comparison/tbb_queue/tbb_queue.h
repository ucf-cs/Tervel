#ifndef TBB_QUEUE_H
#define TBB_QUEUE_H

#include "node.h"
#include "tbb/tbb.h" //concurrent_queue.h"

#include <atomic>
#include <limits.h>

const long kNotValue = LONG_MAX;

class TbbQueue
{
private:
    tbb::concurrent_bounded_queue<Node*> *queue_;
    unsigned long capacity_;
    unsigned int num_threads_;

public:
    void init(unsigned long num_threads, unsigned long capacity);
    bool enqueue(Node *node);
    bool dequeue();
};

#endif  // TBB_QUEUE_H
