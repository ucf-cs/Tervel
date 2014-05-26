#ifndef __slowpath_dec__
#define __slowpath_dec__

#include "wf_macros.hpp"
#include "slowpath.hpp"
#include "WFRingBuffer.hpp"


void EnqueueOp::execute(WFRingBuffer *buffer){//only this op can call it

    EnqueueHelper helper = new EnqueueHelper(this), temp = NULL;
    while (this.ossoc.load() == NULL) {
        // check before getting seq if queue is empty
        if (isFull()) { // return a reference to store the deq val and return if changed
            this.assoc.compare_exchange_strong(temp, (EnqueueHelper *) 0x1);
            return;
        }

        bool back_off = true;
        unsigned long seq = fetchTailSeq(), pos = seq & size_mask_;
        helper->set_seq(seq);
        
        while (this.assoc.load() == NULL) {
            Node *raw_curr_val = (Node *) queue_[pos], *curr_val = raw_curr_val;
            /*
            if (Helper::isHelper(raw_curr_val)) {
                Helper::remove(this, pos, raw_curr_val);
                continue;
            }*/
            if (Node::isMarkedNode(curr_val)) {
                curr_val = Node::unbitmarked(curr_val);
            }

            if (curr_val->seq() == helper->seq()) {
                // enqueue the node
                assert(curr_val->val() == kNotValue);
                if (queue_[pos].compare_exchange_strong(raw_curr_val, helper, 
                                                        std::memory_order_relaxed,
                                                        std::memory_order_relaxed)) {
                    // successful enqueue
                    // Helper is not removed, because it is a node with modified value return
                    this.assoc.compare_exchange_strong(temp, helper);
                    return;
                } 
            } else if (curr_val->seq() < helper->seq()) {
                if (back_off) { // optimistic wait for the dequeue to finish
                    back_off = false;
                    usleep(back_off_);
                    continue;
                } else {
                    if (curr_val->val() == kNotValue) {
                        // Try to take position normally
                        if (queue_[pos].compare_exchange_strong(raw_curr_val, helper, 
                                                                std::memory_order_relaxed,
                                                                std::memory_order_relaxed)) {
                            // successful enqueue
                            // Helper is not removed, because it is a node with modified value return
                            this.assoc.compare_exchange_strong(temp, helper);
                            return;
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
        } // end while (NULL association)
    } // end while (NULL association)
}

void DequeueOp::execute(WFRingBuffer *buffer){

    DequeueHelper helper = new DequeueHelper(this), temp = NULL;
    helper->set_val(kNotValue);
    while (this.assoc.load() == NULL) {
        // check before getting seq if queue is empty
        if (isEmpty()) { // return a reference to store the deq val and return if changed
            this.assoc.compare_exchange_strong(temp, (EnqueueHelper *) 0x1);
            return;
        }

        bool back_off = true;
        unsigned long seq = fetchHeadSeq(), pos = seq & size_mask_;
        helper->set_seq(seq + capacity_);

        while (this.assoc.load() == NULL) {
            Node *raw_curr_val  = (Node *) queue_[pos],  *curr_val = raw_curr_val;
            //Steven: add is helper check
            /*
            if (Helper::isHelper(raw_curr_val)) {
                Helper::remove(this, pos, raw_curr_val);
                continue;
            }*/
            if (Node::isMarkedNode(curr_val)) {
                curr_val = Node::unbitmarked(curr_val);
                helper->set_seq(seq + 2*capacity_);   // this needs to be smarter
            }                                       // can a thread remove a marked helper and replace it with a logical value safely (assumed: Yes)

            if (curr_val->seq() == seq) {
                if (curr_val->val() != kNotValue) {    
                    if (queue_[pos].compare_exchange_strong(raw_curr_val, helper)) {
                        // successful dequeue
                        this.assoc.compare_exchange_strong(temp, helper);
                        return;
                    } else {
                        continue;
                    }
                } else {
                    if (back_off) {
                        back_off = false;
                        usleep(back_off_);
                        continue;
                    } else {
                        helper->set_seq(seq + 2*capacity_);
                        if (queue_[pos].compare_exchange_strong(raw_curr_val, helper)) {
                            this.assoc.compare_exchange_strong(temp, helper);
                            return;
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
                        helper->set_seq(seq + 2*capacity_);
                        if (queue_[pos].compare_exchange_strong(raw_curr_val, helper)) {
                            this.assoc.compare_exchange_strong(temp, helper);
                            return;
                        } else {
                            continue;
                        }
                    } else {
                        void *marked;// = Node::atomic_bitmark(&(queue_[pos]));                               // let it be known this positions seq is behind
                        if (marked == raw_curr_val) { // should this be raw_curr_val                           // but what if another dequeuer with higher seq
                            break;                                                                         // comes around to see the bitmark and sets the
                        }                                                                                  // seq to (seq+2*capacity_) thus making it higher
                    }                                                                                      // than needed. This 'should' only affect performance
                }
            } else { // (seq > curr_val->seq())
                break;
            }
        } // while reexamine position
    }
}

#endif