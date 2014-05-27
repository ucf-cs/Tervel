#ifndef __LINUX_BUFFER_H__
#define __LINUX_BUFFER_H__

#include "node.h"

struct ThreadData{
    volatile unsigned long head;
    volatile unsigned long tail;
};

class LinuxBuffer
{
private:
    Node **queue_;
    ThreadData *thread_data_;
    unsigned long num_threads_;
    unsigned long capacity_;
    unsigned long size_mask_;
    volatile unsigned long head_;
    volatile unsigned long last_head_;
    volatile unsigned long tail_;
    volatile unsigned long last_tail_;
    
public:
    LinuxBuffer() { }
    
    void init(unsigned long num_threads, unsigned long capacity);
    Node* enqueue(Node *node, long thread_id);
    Node* dequeue(long thread_id);
};

#endif  // LINUX_BUFFER_H
