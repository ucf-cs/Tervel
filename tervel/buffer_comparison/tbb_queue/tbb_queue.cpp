#include "tbb_queue.h"
#include "node.h"
#include "tbb/tbb.h"

#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include <bitset>
#include <iostream>

void TbbQueue::init(unsigned long num_threads, unsigned long capacity)
{ 
    capacity_ = capacity;
    num_threads_ = num_threads;

    queue_ = new tbb::concurrent_bounded_queue<Node*>[capacity_];

    if (capacity_ < 2 || (capacity_ & (capacity_-1))) {
        std::cerr << "Capacity was not a power of 2. This may result in errors or " 
                    << "inefficient use of space" << std::endl;
    }
}

bool TbbQueue::enqueue(Node *node)
{
    return queue_->try_push(node);
}

bool TbbQueue::dequeue() // return if val dequeued value but reference the val
{
    Node *val = new Node(kNotValue);
    return queue_->try_pop(val);
}
