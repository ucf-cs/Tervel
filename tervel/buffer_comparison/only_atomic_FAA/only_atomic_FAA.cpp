#include "only_atomic_FAA.h"
#include "node.h"

#include <unistd.h>

#include <bitset>
#include <iostream>

bool OnlyAtomicFAA::enqueue()
{
    fetchTailSeq();
    return true;
}

bool OnlyAtomicFAA::dequeue()
{
    fetchHeadSeq();
    return true;
}

unsigned long OnlyAtomicFAA::fetchHeadSeq() {
    return __sync_fetch_and_add(&head_, 1);
}

unsigned long OnlyAtomicFAA::fetchTailSeq() {
    return __sync_fetch_and_add(&tail_, 1);
}

