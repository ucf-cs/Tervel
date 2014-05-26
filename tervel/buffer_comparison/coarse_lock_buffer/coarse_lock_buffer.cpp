#include "coarse_lock_buffer.h"
#include "node.h"

#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include <bitset>
#include <iostream>

void CoarseLockBuffer::init(unsigned long num_threads, unsigned long capacity)
{ 
    capacity_ = capacity;
    num_threads_ = num_threads;
    size_mask_ = capacity_ - 1;
    head_ = 0;
    tail_ = 0;

    queue_ = new std::atomic<Node*>[capacity_];
    pthread_mutex_init(&queue_lock_, NULL);
    for (unsigned long i = 0; i < capacity_; i++) {
        Node *node = new Node(kNotValue);
        node->set_seq(i);
        queue_[i] = node;
    }

    if (capacity_ < 2 || (capacity_ & (capacity_-1))) {
        std::cerr << "Capacity was not a power of 2. This may result in errors or " 
                    << "inefficient use of space" << std::endl;
    }
}

bool CoarseLockBuffer::enqueue(Node *node)
{
    pthread_mutex_lock(&queue_lock_);
    if (isFull()) {
        pthread_mutex_unlock(&queue_lock_);
        return false;
    } else {
        queue_[fetchHeadSeq() & size_mask_] = node;
        pthread_mutex_unlock(&queue_lock_);
        return true;
    }
}

bool CoarseLockBuffer::dequeue() // return if val dequeued value but reference the val
{
    pthread_mutex_lock(&queue_lock_);
    if (isEmpty()) {
        pthread_mutex_unlock(&queue_lock_);
        return false;
    } else {
        Node *val;
        val = queue_[fetchTailSeq() & size_mask_];
        pthread_mutex_unlock(&queue_lock_);
        return true;
    }
}

unsigned long CoarseLockBuffer::fetchHeadSeq() {
    return head_++;
    //return __sync_fetch_and_add(&head_, 1);
}

unsigned long CoarseLockBuffer::fetchTailSeq() {
    return tail_++;
    //return __sync_fetch_and_add(&tail_, 1);
}

bool CoarseLockBuffer::isFull() {
    //return (tail_ > head_+size_mask_);
    return (tail_ == head_+capacity_);
}

bool CoarseLockBuffer::isEmpty() {
    //return (head_ >= tail_);
    return (head_ == tail_);
}
