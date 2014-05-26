#ifndef __BUFFER_HELPING__
#define __BUFFER_HELPING__ 1

#include "wf_ring_buffer.hpp"
#include "node.h"

#include "slowpath.hpp"

#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include <bitset>
#include <iostream>


/**
 *  Simple notes for now:
 *  
 *      returns true if the thread succeeded in the operation
 *      sets the OpRec val to NULL if enqueued if enqueue occurred
 *          -successful enqueue denoted (return: true | op->val: NULL)
 *          -full denoted by (return: true | op->val: [untouched])
 *          -maxfails reached by (return: false)
 */
 //Steven: Did not review for now.
bool WFRingBuffer::helpEnqueue(void *op)
{
    /*
    int retry_count=0;
    Node *node = op->val;
    while (retry_count < max_retry_) {
        // check before getting seq if queue is empty
        if (isFull()) { // return a reference to store the deq val and return if changed
            //node = (Node *) 0x1;
            return true;
        }

        bool back_off = true;
        unsigned long seq = fetchTailSeq(), pos = seq & size_mask_;
        node->set_seq(seq);
        
        while (true) {
            Node *raw_curr_val = queue_[pos], *curr_val = raw_curr_val;
            if (Node::isMarkedNode(curr_val)) {
                curr_val = Node::unbitmarked(curr_val);
            }

            if (curr_val->seq() == op->val->seq()) {
                // enqueue the node
                if (queue_[pos].compare_exchange_strong(raw_curr_val, node, 
                                                        std::memory_order_relaxed,
                                                        std::memory_order_relaxed)) {
                    // successful enqueue
                    node = (Node *) 0x0;
                    return true;
                } 
            } else if (curr_val->seq() < node->seq()) {
                if (back_off) { // optimistic wait for the dequeue to finish
                    back_off = false;
                    usleep(back_off_);
                    continue;
                } else {
                    if (curr_val->val() == kNotValue) {
                        // Try to take position normally
                        if (queue_[pos].compare_exchange_strong(raw_curr_val, node, 
                                                                std::memory_order_relaxed,
                                                                std::memory_order_relaxed)) {
                            // successful enqueue
                            node = (Node *) 0x0;
                            return true;
                        } else { // some other thread enqueued, find a new location
                            break; 
                        }
                    } else { // curr_val->val() != kNotValue
                        // Seq number is < expected && dequeuer hasnt removed an element
                        // thus, get a new seq number
                        break;
                    }
                }
            } else {    // curr_val->seq() > node->seq()
                // thus, get a new seq number
                break;
            }
        } 

        // another thread must have invalidated the position 
        retry_count++;
    }
    
    node = (Node *) 0x2;
    */
    return false;
}

/**
 *  Simple notes for now:
 *  
 *      returns true if the thread succeeded in the operation
 *      sets the OpRec val to the val dequeued, if dequeue occurred
 *          -successful dequeue denoted (return: true | op->val: Val)
 *          -empty denoted by (return: true | op->val: NULL)
 *          -maxfails reached by (return: false)
 */
bool WFRingBuffer::helpDequeue(void *op)
{
    /*
    Node *node = new Node(kNotValue);
    int retry_count=0;
    while (retry_count < max_retry_) {
        // check before getting seq if queue is empty
        if (isEmpty()) { // return a reference to store the deq val and return if changed
            op->val = (Node *) 0x1;
            return true;
        }

        bool back_off = true;
        unsigned long seq = fetchHeadSeq(), pos = seq & size_mask_;
        node->set_seq(seq + capacity_);

        while (true) {
            Node *raw_curr_val = queue_[pos], *curr_val = raw_curr_val;
            if (Node::isMarkedNode(curr_val)) {
                curr_val = Node::unbitmarked(curr_val);
                node->set_seq(seq + 2*capacity_);   // this needs to be smarter
            }

            if (curr_val->seq() == seq) {
                if (curr_val->val() != kNotValue) {    
                    if (queue_[pos].compare_exchange_strong(raw_curr_val, node)) {
                        // successful dequeue
                        op->val = curr_val;
                        return true;
                    } else {
                        continue;
                    }
                } else {
                    if (back_off) {
                        back_off = false;
                        usleep(back_off_);
                        continue;
                    } else {
                        node->set_seq(seq + 2*capacity_);
                        if (queue_[pos].compare_exchange_strong(raw_curr_val, node)) {
                            break;
                        } else {
                            continue;
                        }
                    }
                }
            } else if (curr_val->seq() < seq) {
                if (back_off) {
                    back_off = false;
                    usleep(back_off_);
                    continue;
                } else {
                    if (curr_val->val() == kNotValue) {
                        node->set_seq(seq + 2*capacity_);
                        if (queue_[pos].compare_exchange_strong(raw_curr_val, node)) {
                            break;
                        } else {
                            continue;
                        }
                    } else {
                        void *marked = Node::atomic_bitmark(&(queue_[pos])); // let it be known this positions seq is behind
                        if (marked == raw_curr_val) {                            // but what if another dequeuer with higher seq
                            break;                                           // comes around to see the bitmark and sets the
                        }                                                    // seq to (seq+2*capacity_) thus making it higher
                    }                                                        // than needed. This 'should' only affect performance
                }
            } else { // (seq > curr_val->seq())
                break;
            }
        } // while reexamine position
        
        retry_count++;
    }

    op->val = (Node *) 0x2;
    */
    return false;
}

/*
void WFRingBuffer::announceOp(void *op) {
    /*
    assert(op_table_[thread_id] == NULL);
    op_table_[thread_id] = op;
    *
}

void WFRingBuffer::removeOp(unsigned long thread_id) {
    /*
    assert(op_table_[thread_id] != NULL);
    op_table_[thread_id] = NULL;
    *
}
*/

#endif
