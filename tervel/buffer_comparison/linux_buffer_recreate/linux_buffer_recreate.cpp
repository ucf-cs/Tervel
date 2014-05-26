#include "linux_buffer_recreate.h"
#include "node.h"

#include <limits.h>
#include <bitset>
#include <iostream>

void LinuxBuffer::init(unsigned long num_threads, unsigned long capacity)
{ 
    capacity_ = capacity;
    num_threads_ = num_threads;
    size_mask_ = capacity_ - 1;
    head_ = 0;
    tail_ = 0;
    last_head_ = 0;
    last_tail_ = 0;

    queue_ = new Node*[capacity_];
    thread_data_ = new ThreadData[num_threads_];
    for (unsigned long i = 0; i < num_threads_; i++) {
        thread_data_[i].head = ULONG_MAX;
        thread_data_[i].tail = ULONG_MAX;
    }

    if (capacity_ < 2 || (capacity_ & (capacity_-1))) {
        std::cerr << "Capacity was not a power of 2. This may result in errors or " 
                    << "inefficient use of space" << std::endl;
    }
}

Node* LinuxBuffer::enqueue(Node *node, long thread_id)
{
    thread_data_[thread_id].tail = __sync_fetch_and_add(&tail_, 1); 

    while (thread_data_[thread_id].tail >= last_head_+capacity_) {
        unsigned long min_head = head_;
        
        //std::cout << "Updating head " << thread_id << std::endl;
        
        for (unsigned long i = 0; i < num_threads_; i++) {
            volatile unsigned long tmp = thread_data_[thread_id].head;

            if (tmp < min_head)
                min_head = tmp;
        }
        last_head_ = min_head;
    }
        
    queue_[thread_data_[thread_id].tail & size_mask_] = node;
    thread_data_[thread_id].tail = ULONG_MAX;

    return node;
}

Node* LinuxBuffer::dequeue(long thread_id)
{
    thread_data_[thread_id].head = __sync_fetch_and_add(&head_, 1);
    Node *ret;
    
    while (thread_data_[thread_id].head >= last_tail_) {
        unsigned long min_tail = tail_;
        
        //std::cout << "Updating tail " << thread_id  << std::endl;

        for (unsigned long i = 0; i < num_threads_; i++) {
            volatile unsigned long tmp = thread_data_[thread_id].tail;

            if (tmp < min_tail)
                min_tail = tmp;
        }
        last_tail_ = min_tail;
    }
    
    ret = queue_[thread_data_[thread_id].head & size_mask_];
    thread_data_[thread_id].head = ULONG_MAX;
    return ret;
}
