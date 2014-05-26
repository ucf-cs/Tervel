#include "linux_buffer.h"
#include "node.h"
#include "lockfree_rb_q.h"

#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include <bitset>
#include <iostream>

void LinuxBuffer::init(unsigned long num_threads, unsigned long capacity)
{ 
    capacity_ = capacity;
    num_threads_ = num_threads;
    size_mask_ = capacity_ - 1;
    head_ = 0;
    tail_ = 0;

    //queue_ = new std::atomic<Node*>[capacity_];
    queue_ = new LockFreeQueue(num_threads/2, num_threads/2);

    if (capacity_ < 2 || (capacity_ & (capacity_-1))) {
        std::cerr << "Capacity was not a power of 2. This may result in errors or " 
                    << "inefficient use of space" << std::endl;
    }
}

Node* LinuxBuffer::enqueue(Node *node)
{
    queue_[fetchHeadSeq() & size_mask_] = node;
    return node;
}

Node* LinuxBuffer::dequeue() // return if val dequeued value but reference the val
{
    Node *val;
    val = queue_[fetchTailSeq() & size_mask_];
    return val;
}
