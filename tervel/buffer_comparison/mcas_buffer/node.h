#ifndef NODE_H
#define NODE_H

#include <atomic>
#include <cstddef>

class Node 
{
private:
    unsigned long seq_;
    long val_;      // if NULL this is valid insert pos; else valid dequeue pos

public:
    //Node() { val_ = kNotValue; }
    Node(long val) { val_ = val; }

    unsigned long seq() const { return seq_; }
    void set_seq(unsigned long const seq) { seq_ = seq; }
    long val() const { return val_; }
    void set_val(unsigned long const val) { val_ = val; }
    static void* atomic_bitmark(std::atomic<Node*> *loc) 
    { 
        std::atomic_ptrdiff_t *temp = (std::atomic_ptrdiff_t*) loc; 
        return (void*) temp->fetch_or(0x1);
    }
    static Node* unbitmarked(void *p) { return (Node*) ((long) p & (~0x1)); }
    static bool isMarkedNode(void *p) { return 1 == ((long) p & 1); }
};


#endif  // NODE_H
