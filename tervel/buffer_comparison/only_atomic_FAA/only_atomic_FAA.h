#ifndef ONLY_ATOMIC_FAA_H
#define ONLY_ATOMIC_FAA_H

#include "node.h"
#include <limits.h>
#include <atomic>


class OnlyAtomicFAA
{
private:
    volatile unsigned long head_;
    volatile unsigned long tail_;
    volatile unsigned long back_off_;

    unsigned long fetchHeadSeq();
    unsigned long fetchTailSeq();
public:
    OnlyAtomicFAA() { }

    bool enqueue();
    bool dequeue();
};

#endif  // WF_RING_BUFFER_H
