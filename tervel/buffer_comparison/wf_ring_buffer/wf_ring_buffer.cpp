#include "node.h"

#include "wf_ring_buffer.hpp"
//#include "slowpath.hpp"
#include "wf_ring_buffer_helping.hpp"
#include "wf_ring_buffer_announce.hpp"

#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include <bitset>
#include <iostream>

#define SLOW_PATH_ONLY

void WFRingBuffer::init(unsigned long num_threads, unsigned long capacity)
{ 
    capacity_ = capacity;
    num_threads_ = num_threads;
    size_mask_ = capacity_ - 1;
    head_ = 0;
    tail_ = 0;
    max_retry_ = 3;
    //max_fail_ = 3;
    back_off_ = 10;

    opTable = new std::atomic<void*>[num_threads_];
    //helpers_ = new std::atomic<Helper*>[num_threads_];//Steven: whats this for?
    for (unsigned long i = 0; i < num_threads_; i++) {
//        opTable[i].store((void *) 0x0);
        opTable[i].store(nullptr);
        //helpers_[i] = new Helper(); //Steven: I am worrried...
        //(helpers_[i]).load()->check_id = i; //Steven: not sure why...
    }

    //queue_ = new std::atomic<Node*>[capacity_];
    queue_ = new std::atomic<Node*>[capacity_];
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

Node* WFRingBuffer::enqueue(unsigned long thread_id, Node *node)
{
    checkOpTable();

    int retry_count=0;
    while (retry_count < max_retry_) {
        // check before getting seq if queue is empty
        if (isFull()) { // return a reference to store the deq val and return if changed
            return (Node *) 0x1;
        }

        bool back_off = true;
        unsigned long seq = fetchTailSeq(), pos = seq & size_mask_;
        node->set_seq(seq);
        
        while (true) {
            //Steven: need check if it is a helper.
            Node *raw_curr_val = (Node *) queue_[pos].load();
            Node *curr_val = raw_curr_val;
            /*if (Helper::isHelper(raw_curr_val)) {
                Helper::remove(this, pos, raw_curr_val);
                continue;
            }*/
            if (Node::isMarkedNode(curr_val)) {
                curr_val = Node::unbitmarked(curr_val);
            }

            if (curr_val->seq() == node->seq()) {
                // enqueue the node
                assert(curr_val->val() == kNotValue);

                std::atomic<Node *> * spot=&(queue_[pos]);
                if (spot->compare_exchange_strong(raw_curr_val, node)) {
                    // successful enqueue
                    return node;
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
                            return node;
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
    
    EnqueueOp *op = new EnqueueOp(node->val());
    announceOp(op);
    return op->getResult();
}

Node* WFRingBuffer::dequeue(unsigned long thread_id)
{
    //tryHelpAnother(thread_id);
    checkOpTable();

    Node *node = new Node(kNotValue);
    int retry_count=0;
    while (retry_count < max_retry_) {
        // check before getting seq if queue is empty
        if (isEmpty())  // return a reference to store the deq val and return if changed
            return (Node *) 0x1;

        bool back_off = true;
        unsigned long seq = fetchHeadSeq(), pos = seq & size_mask_;
        node->set_seq(seq + capacity_);

        while (true) {
            Node *raw_curr_val = (Node *) queue_[pos], *curr_val = raw_curr_val;
            //Steven: add is helper check
            /*
            if (Helper::isHelper(raw_curr_val)) {
                Helper::remove(this, pos, raw_curr_val);
                continue;
            }*/
            if (Node::isMarkedNode(curr_val)) {
                curr_val = Node::unbitmarked(curr_val);
                node->set_seq(seq + 2*capacity_);   // this needs to be smarter
            }                                       // can a thread remove a marked helper and replace it with a logical value safely (assumed: Yes)

            if (curr_val->seq() == seq) {
                if (curr_val->val() != kNotValue) {
                    if (queue_[pos].compare_exchange_strong(raw_curr_val, node)) {
                 //       // successful dequeue
                        return curr_val;
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
        
        retry_count++;
    }
    
    DequeueOp *op = new DequeueOp();
    announceOp(op);
    node = op->getResult();
    return node;
}

unsigned long WFRingBuffer::fetchHeadSeq() {
    return __sync_fetch_and_add(&head_, 1);
}

unsigned long WFRingBuffer::fetchTailSeq() {
    return __sync_fetch_and_add(&tail_, 1);
}

bool WFRingBuffer::isFull() {
    return (tail_ > head_+size_mask_);
}

bool WFRingBuffer::isEmpty() {
    return (head_ >= tail_);
}

void WFRingBuffer::tryHelpAnother(unsigned long thread_id) {
    
    /*
    //Steven: why did you make an object? To heavy handed/requires an array.
    //Use __thread long check_id = ... instead
    //same for storing the threads id.
    //Helpers should not be re-used in this manner. It may  not safe. 
    Helper *helper = helpers_[thread_id].load(); //Steven: unnecssary loads are costly
    helper->check_id = (helper->check_id+1)%num_threads_;
    helper->op = op_table_[helper->check_id].load();////Steven: no need to be storing this info globally

    ////Steven: OpRecord should have a "complete function"
    //So instead of the bellow you simply have if not null op->complete().
    //Steven: encapsulation makes for better maintiability.
    //
    if (helper->op != NULL && (helper->op->isPending)) {
        Helper *curr_helper = helper->op->helper;
        // check if another thread is helping
        if (curr_helper != NULL) { // another thread is already helping
            return;//Steven: op must be done in this case. 
        }
        
        //Steven: NO, you must place the helper into the queue then associate it. If the associ == success replace with result otherwise replace with failure...
        // try to associate with the OpRec
        if (!((helper->op->helper).compare_exchange_strong(curr_helper, helper))) {
            return; // failed to associate, someone else is helping
        }

        if (helper->op->isEnq) { // && helper->op->val->val() != kNotValue) {
            //std::cout << "enq" << std::endl;
            if (helpEnqueue(helper->op))
                helper->op->isPending = false;
        } else {  // if (helper->op->val->val() == kNotValue) {
            //std::cout << "deq" << std::endl;
            if (helpDequeue(helper->op))
                helper->op->isPending = false;
        }

        if ((helper->op->helper).compare_exchange_strong(helper, (Helper *) 0x0)) {
            return;
        } else {
            assert(false);
        }
    }
    */

}

/**
 *  Simple notes for now:
 *  
 *      returns true if the thread succeeded in the operation
 *      sets the OpRec val to NULL if enqueued if enqueue occurred
 *          -successful enqueue denoted (return: true | op->val: NULL)
 *          -full denoted by (return: true | op->val: [untouched])
 *          -maxfails reached by (return: false)
 *
 //Steven: Did not review for now.
bool WFRingBuffer::helpEnqueue(EnqueueOp *op)
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
 *
bool WFRingBuffer::helpDequeue(DequeueOp *op)
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
    return false;
}

void WFRingBuffer::announceOp(unsigned long thread_id, OpRecord *op) {
    /*
    assert(op_table_[thread_id] == NULL);
    op_table_[thread_id] = op;
} 

void WFRingBuffer::removeOp(unsigned long thread_id) {
    /*
    assert(op_table_[thread_id] != NULL);
    op_table_[thread_id] = NULL;
} */

//#include "wf_ring_buffer_helping.hpp"
