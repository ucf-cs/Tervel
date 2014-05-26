#ifndef COARSE_LOCK_BUFFER_H
#define COARSE_LOCK_BUFFER_H

#include "node.h"
#include <limits.h>
#include <pthread.h>

#include <atomic>

const long kNotValue = LONG_MAX;

class CoarseLockBuffer
{
private:
    std::atomic<Node*> *queue_;
    unsigned int num_threads_;
    unsigned long capacity_;
    unsigned long size_mask_;
    volatile unsigned long head_;
    volatile unsigned long tail_;
    pthread_mutex_t queue_lock_;
    
    unsigned long fetchHeadSeq();
    unsigned long fetchTailSeq();
    bool isFull();
    bool isEmpty();
public:
    void init(unsigned long num_threads, unsigned long capacity);
    bool enqueue(Node *node);
    bool dequeue();
};

#endif  // COARSE_LOCK_BUFFER_H
