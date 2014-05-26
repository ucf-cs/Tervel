#ifndef LINUX_BUFFER_H
#define LINUX_BUFFER_H

#include "node.h"
#include <limits.h>
#include <pthread.h>

#include <atomic>

const long kNotValue = LONG_MAX;

class LinuxBuffer
{
private:
    std::atomic<Node*> *queue_;
    unsigned int num_threads_;
    unsigned long capacity_;
public:
    void init(unsigned long num_threads, unsigned long capacity);
    Node* enqueue(Node *node);
    Node* dequeue();
};

#endif  // COARSE_LOCK_BUFFER_H
