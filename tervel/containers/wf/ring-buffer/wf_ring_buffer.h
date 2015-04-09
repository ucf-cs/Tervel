/*
The MIT License (MIT)

Copyright (c) 2015 University of Central Florida's Computer Software Engineering
Scalable & Secure Systems (CSE - S3) Lab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef TERVEL_WFRB_RINGBUFFER_H_
#define TERVEL_WFRB_RINGBUFFER_H_

#ifdef DEBUG
  #include <tbb/concurrent_hash_map.h>
#endif

#include <tervel/util/info.h>
#include <tervel/util/system.h>
#include <tervel/util/padded_atomic.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/rc/descriptor_util.h>

#include <tervel/containers/wf/ring-buffer/node.h>
#include <tervel/containers/wf/ring-buffer/empty_node.h>
#include <tervel/containers/wf/ring-buffer/elem_node.h>
#include <tervel/containers/wf/ring-buffer/enqueue_op.h>
#include <tervel/containers/wf/ring-buffer/dequeue_op.h>


#include <stdlib.h>

#include <unistd.h>

#include <atomic>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <type_traits>

namespace tervel {
namespace wf_ring_buffer {

template<class T>
class EnqueueOp;
template<class T>
class DequeueOp;
template<class T>
class Node;
template<class T>
class ElemNode;
template<class T>
class EmptyNode;

/**
 * TODO class desc
 */
template<class T>
class RingBuffer : public util::memory::hp::Element {
 public:
  static constexpr T FAIL_CONST = reinterpret_cast<T>(0x1L);

  explicit RingBuffer<T>(size_t capacity)
      : capacity_(capacity)
      , size_mask_(capacity_ - 1)
      , buffer_(new util::PaddedAtomic<Node<T> *>[capacity_]) {
    assert(capacity_ != 0);
    for (int64_t i = 0; i < capacity_; i++) {
      Node<T> *empty_node = util::memory::rc::get_descriptor< EmptyNode<T> >(i);
      buffer_[i].store(empty_node);
    }
  }

  ~RingBuffer<T>() {}

  /**
   * TODO: Enqueues buffer element...
   */
  bool enqueue(T val);

  /**
   * TODO: Dequeues buffer element...
   */
  bool dequeue(T &result);

  /**
   * @return whether the buffer is empty
   */
  bool is_empty();

  /**
   * @return whether the buffer is full
   */
  bool is_full();


  /**
   * METHODS BEYOND THIS POINT ARE FOR TESTING PURPOSE ONLY
   */
  #if DEBUG
  void print_content_count() __attribute__((used));
  void print_lost_nodes() __attribute__((used));

  #endif  // DEBUG

  const int capacity()  {return capacity_;}
private:
  /**
   * TODO: Performs an enqueue..
   */
  bool lf_enqueue(T value);

  /**
   * TODO: Performs a dequeue...
   */
  bool lf_dequeue(T *result);

  // REVIEW(steven) missing description
  void wf_enqueue(EnqueueOp<T> *op);

  // REVIEW(steven) missing description
  void wf_dequeue(DequeueOp<T> *op);

  // REVIEW(steven) missing description
  int64_t next_head_seq();

  // REVIEW(steven) missing description
  int64_t next_tail_seq();

  // REVIEW(steven) missing description
  int64_t get_head_seq();

  // REVIEW(steven) missing description
  int64_t get_tail_seq();

  // REVIEW(steven) missing description
  int64_t get_position(int64_t seq);

  const int capacity_;
  const int size_mask_;
  std::unique_ptr<util::PaddedAtomic<Node<T> *>[]> buffer_;


  util::PaddedAtomic<int64_t> head_ {0};
  util::PaddedAtomic<int64_t> tail_ {0};

  #if DEBUG
  std::map<T, int> log_dequeue;
  std::map<T, int> log_enqueue;
  std::mutex log_mutex_enqueue;
  std::mutex log_mutex_dequeue;
  #endif  // DEBUG

  friend DequeueOp<T>;
  friend EnqueueOp<T>;
};  // RingBuffer class


template<class T>
bool RingBuffer<T>::enqueue(T value) {
  util::ProgressAssurance::check_for_announcement();
  bool succ = lf_enqueue(value);
  #if DEBUG
  if (succ) {
    log_mutex_enqueue.lock();
    int temp = log_enqueue[value];
    assert(temp == 0);
    log_enqueue[value]++;
    log_mutex_enqueue.unlock();
  }
  #endif  // DEBUG
  return succ;
}

template<class T>
bool RingBuffer<T>::dequeue(T &result) {
  util::ProgressAssurance::check_for_announcement();
  bool succ = lf_dequeue(&result);
  #if DEBUG
  if (succ) {
    log_mutex_dequeue.lock();
    int temp = log_dequeue[result];
    assert(temp == 0);
    log_dequeue[result]++;
    log_mutex_dequeue.unlock();
  }
  #endif  // DEBUG
  return succ;
}

template<class T>
bool RingBuffer<T>::lf_enqueue(T val) {
  util::ProgressAssurance::Limit progAssur;

  while (true) { // REVIEW(steven) describe loop
    if (is_full()) {
      return false;
    }

    int64_t seq = next_tail_seq();
    int64_t pos = get_position(seq);

    // REVIEW(steven) describe loop
    while (true) {
      if (progAssur.isDelayed()) {
        EnqueueOp<T> *op = new EnqueueOp<T>(this, val);
        util::ProgressAssurance::make_announcement(
              reinterpret_cast<tervel::util::OpRecord *>(op));
        bool result = op->result();
        op->safe_delete();
        return result;
      }

      Node<T> *curr_node = buffer_[pos].load(std::memory_order_relaxed);
      Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
            util::memory::rc::unmark_first(curr_node));
      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
            reinterpret_cast<std::atomic<void *> *>(&(buffer_[pos].atomic)),
            curr_node);

      if (!watch_succ) {
        continue;
      }
      else if (curr_node != unmarked_curr_node) {  // curr_node is skipped marked
        // Enqueues do not modify marked nodes
        util::memory::rc::unwatch(unmarked_curr_node);
        break;
      } else {  // curr_node isnt marked skipped
        if (curr_node->seq() < seq) {
            util::backoff();
            if (curr_node != buffer_[pos].load()) {  // curr_node has changed
              util::memory::rc::unwatch(unmarked_curr_node);
              continue;  // reprocess the current value.
            }
        }

        if (curr_node->seq() <= seq && curr_node->is_EmptyNode()) {
          Node<T> *new_node = util::memory::rc::get_descriptor< ElemNode<T> >(
                val, seq);

          bool cas_success = buffer_[pos].compare_exchange_strong(
                curr_node, new_node);

          if (cas_success) {
            util::memory::rc::unwatch(unmarked_curr_node);
            util::memory::rc::free_descriptor(unmarked_curr_node);
            return true;
          } else {
            util::memory::rc::unwatch(unmarked_curr_node);
            util::memory::rc::free_descriptor(new_node, true);
            break;
          }
        } else {  // (curr_node->seq() > seq) {
          util::memory::rc::unwatch(unmarked_curr_node);
          break;
        }
      }  // else (curr_node isnt marked skipped)
    }  // while (true)
  }  // while (true)
}  // lf_enqueue(T val)

template<class T>
bool RingBuffer<T>::lf_dequeue(T *result) {
  util::ProgressAssurance::Limit progAssur;
  while (true) { // REVIEW(steven) describe loop
    if (is_empty()) {
      return false;
    }

    int64_t seq = next_head_seq();
    int64_t pos = get_position(seq);
    while (true) { // REVIEW(steven) describe loop
      if (progAssur.isDelayed()) {

        DequeueOp<T> *op = new DequeueOp<T>(this);
        util::ProgressAssurance::make_announcement(
              reinterpret_cast<tervel::util::OpRecord *>(op));
        bool op_succ = op->result(result);
        op->safe_delete();
        return op_succ;
      }

      Node<T> *curr_node = buffer_[pos].load(std::memory_order_relaxed);
      Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
            util::memory::rc::unmark_first(curr_node));

      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
            reinterpret_cast<std::atomic<void*> *>(&(buffer_[pos].atomic)),
            curr_node);

      if (!watch_succ) {
        continue;
      }
      if (curr_node != unmarked_curr_node) {  // curr_node is marked skipped
        if (unmarked_curr_node->is_EmptyNode()) {
          Node<T> *new_node = util::memory::rc::get_descriptor<EmptyNode<T>>(
                seq + capacity_);

          bool cas_success = buffer_[pos].compare_exchange_strong(curr_node,
                                                                  new_node);
          if (cas_success) {
            util::memory::rc::free_descriptor(unmarked_curr_node);
            util::memory::rc::unwatch(unmarked_curr_node);
            break;
          } else {
            util::memory::rc::free_descriptor(new_node, true);
            util::memory::rc::unwatch(unmarked_curr_node);
            continue;
          }
        } else {  // curr_node is ElemNode and Skipped Marked

          if (unmarked_curr_node->seq() == seq) {
            // We must mark the node as out of sync
            Node<T> *new_node = util::memory::rc::get_descriptor< EmptyNode<T> >(
                  seq + capacity_);
            Node<T> *marked_new_node = reinterpret_cast<EmptyNode<T> *>(
                  tervel::util::memory::rc::mark_first(new_node));

            bool cas_success = buffer_[pos].compare_exchange_strong(curr_node,
                  marked_new_node);

            if (cas_success) {
              util::memory::rc::unwatch(unmarked_curr_node);
              util::memory::rc::free_descriptor(unmarked_curr_node);

              *result = unmarked_curr_node->val();
              return true;
            } else {
              util::memory::rc::unwatch(unmarked_curr_node);
              util::memory::rc::free_descriptor(new_node, true);
              continue;
            }
          } else {
            // REVIEW(steven): explain this break statement
            util::memory::rc::unwatch(unmarked_curr_node);
            break;
          }
        }
      } else {  // curr_node isnt marked skipped
        if (unmarked_curr_node->seq() == seq) {
          if (unmarked_curr_node->is_ElemNode()) {
            // The current node is an ElemNode and we are assigned its seq
            // number. We also are sure it is not associated with an OpRecord.
            // However, this node may be removed by a copy with an OpRecord.
            // Since no other thread can remove the value, we CAS to be able
            // to determine if the position has been bitmarked..
            Node<T> *new_node = util::memory::rc::get_descriptor< EmptyNode<T> >(
                  seq + capacity_);

            bool cas_succ = buffer_[pos].compare_exchange_strong(
                  curr_node, new_node);

            if (cas_succ) {
              *result = curr_node->val();
              util::memory::rc::unwatch(unmarked_curr_node);
              util::memory::rc::free_descriptor(unmarked_curr_node);
              return true;
            } else {
              // The value may have been bitmarked or been replaced by a copy
              // with an OpRecord
              util::memory::rc::unwatch(unmarked_curr_node);
              util::memory::rc::free_descriptor(new_node, true);
              continue;
            }
          }  // if is elemnode

        } else if (curr_node->seq() > seq) {
          util::memory::rc::unwatch(unmarked_curr_node);
          break;
        }
        // Otherwise, (curr_node->seq() < seq) and we must set skipped if no
        // progress occurs after backoff
        util::backoff();
        if (curr_node == buffer_[pos].load()) {
          util::memory::rc::atomic_mark_first(
                reinterpret_cast<std::atomic<void*> *>(&(buffer_[pos]).atomic));
        }
        util::memory::rc::unwatch(unmarked_curr_node);
        continue;
      }  // else (is not skipped)
      assert(false);
    }  // while (true)
  }  // while (true)
}  // lf_dequeue(T result)

template<class T>
void RingBuffer<T>::wf_enqueue(EnqueueOp<T> *op) {
  int64_t seq = get_tail_seq() - 1;

  while (true) {
    if (is_full()) {
      op->try_set_failed();
      assert(op->helper_.load() != nullptr);
      return;
    }

    seq++;
    int64_t pos = get_position(seq);

    while (op->helper_.load() == nullptr) {
      Node<T> *curr_node = buffer_[pos].load(std::memory_order_relaxed);
      Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
            util::memory::rc::unmark_first(curr_node));

      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
            reinterpret_cast<std::atomic<void*> *>(&(buffer_[pos].atomic)),
            curr_node);

      if (!watch_succ) {
        continue;
      }
      if (curr_node != unmarked_curr_node) {  // curr_node is marked skipped
        // Enqueues do not modify marked nodes
        util::memory::rc::unwatch(unmarked_curr_node);
        break;
      } else {  // curr_node isnt marked skipped
        if (unmarked_curr_node->seq() < seq) {
          util::backoff();
          if (curr_node != buffer_[pos].load()) {  // curr_node has changed
            util::memory::rc::unwatch(unmarked_curr_node);
            continue;  // re-process the current value
          }  // curr_node changed during backoff
        }

        if (curr_node->seq() <= seq && curr_node->is_EmptyNode()) {
          ElemNode<T> *new_node = util::memory::rc::get_descriptor<ElemNode<T>>(
                op->value(), seq,  op);

          bool cas_success = buffer_[pos].compare_exchange_strong(curr_node,
                new_node);

          if (cas_success) {
            util::memory::rc::unwatch(unmarked_curr_node);
            util::memory::rc::free_descriptor(unmarked_curr_node);
            op->associate(new_node, &(buffer_[pos].atomic));
            assert(op->helper_.load() != nullptr);
            return;
          } else {  // failed to place the new node
            util::memory::rc::unwatch(unmarked_curr_node);
            util::memory::rc::free_descriptor(new_node, true);
          }
        } else {  // (curr_node->seq() > seq)
          util::memory::rc::unwatch(unmarked_curr_node);
          break;  // break inner loop and get a new sequence number
        }
      }  // else (curr_node isnt marked skipped)
    }  // while (true)
  }  // while (true)
}  // wf_enqueue()

template<class T>
void RingBuffer<T>::wf_dequeue(DequeueOp<T> *op) {
  int64_t seq = get_head_seq() - 1;

  while (true) {
    if (is_empty()) {
      op->try_set_failed();
      assert(op->helper_.load() != nullptr);
      return;
    }

    seq++;
    int64_t pos = get_position(seq);

    while (op->helper_.load() == nullptr) {
      Node<T> *curr_node = buffer_[pos].load(std::memory_order_relaxed);
      Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
            util::memory::rc::unmark_first(curr_node));

      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
            reinterpret_cast<std::atomic<void *> *>(&(buffer_[pos].atomic)),
            reinterpret_cast<void *>(curr_node));

      if (!watch_succ) {
        continue;
      }
      if (curr_node != unmarked_curr_node) {  // curr_node is marked skipped
        if (unmarked_curr_node->is_EmptyNode()) {
          /*Node<T> *new_node = reinterpret_cast<Node<T> *>(
              util::memory::rc::get_descriptor<EmptyNode<T>>(seq + capacity_));

          bool cas_success = buffer_[pos].compare_exchange_strong(curr_node,
                                                                  new_node);
          if (cas_success) {
            util::memory::rc::free_descriptor(unmarked_curr_node);
            break;
          } else {
            util::memory::rc::free_descriptor(new_node, true);
            continue;
          } */
            // If the thread was delayed after getting ths position, but before
            // dereferencing the current node, then the above could allow an old
            // seqid to be reused. Which is bad!
            //
            // For now, lets just get a new position and mellow on how to handle
            // this
            //
            util::memory::rc::unwatch(unmarked_curr_node);
            break;
        } else {  // curr_node is ElemNode
          // curr_node is a skipped ElemNode. It must be removed by a thread
          // with the values seq number so it may be corrected.
          util::memory::rc::unwatch(unmarked_curr_node);
          break;
        }
      } else {  // curr_node isnt marked skipped
        if (unmarked_curr_node->seq() == seq) {
          if (unmarked_curr_node->is_ElemNode()) {
            Node<T> *new_node = util::memory::rc::get_descriptor<
                  ElemNode<T> >(unmarked_curr_node->val(), seq, op);

            bool cas_succ = buffer_[pos].compare_exchange_strong(
                  curr_node, new_node);
            if (cas_succ) {
              util::memory::rc::unwatch(unmarked_curr_node);
              util::memory::rc::free_descriptor(unmarked_curr_node);
              op->associate(reinterpret_cast<ElemNode<T> *>(new_node),
                    &(buffer_[pos].atomic));
              return;
            } else {
              util::memory::rc::unwatch(unmarked_curr_node);
              util::memory::rc::free_descriptor(new_node, true);
              continue;
            }
          }

        } else if (curr_node->seq() > seq) {
          util::memory::rc::unwatch(unmarked_curr_node);
          break;
        }
        // Otherwise, (curr_node->seq() < seq) and do not set skipped if no
        // progress occurs after backoff because we were not assigned a sequence
        // number
        util::backoff();
        if (curr_node == buffer_[pos].load()) {
          util::memory::rc::unwatch(unmarked_curr_node);
          break;
        }
        util::memory::rc::unwatch(unmarked_curr_node);
      }  // else (is not skipped)
    }  // while (true)
  }  // while (true)
}  // wf_dequeue(DequeueOp *op)

template <class T>
bool RingBuffer<T>::is_empty() {
  return head_.load() >= tail_.load();
}

template <class T>
bool RingBuffer<T>::is_full() {
  return tail_.load() >= head_.load()+capacity_;
}

#if DEBUG

template <class T>
void RingBuffer<T>::print_content_count() {
  int num_elem = 0, num_empty = 0;
  int m_num_elem = 0, m_num_empty = 0;
  for (int i=0; i<capacity_; i++) {
    Node<T> *curr_node = buffer_[i].load();
    Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
          util::memory::rc::unmark_first(curr_node));
    if (unmarked_curr_node->is_EmptyNode()) {
      if (curr_node == unmarked_curr_node) {
        num_empty++;
      }
      else {
        m_num_empty++;
      }
    } else {
      if(curr_node == unmarked_curr_node){
        num_elem++;
      }
      else {
        m_num_empty++;
      }
    }
  }
  printf("%s\n", (num_empty==0) ? "True" : "False");
  printf("Unmarked: ElemNode count: %d\tEmptyNode count: %d\n", num_elem, num_empty);
  printf("Marked:   ElemNode count: %d\tEmptyNode count: %d\n", m_num_elem, m_num_empty);
}


template <class T>
void RingBuffer<T>::print_lost_nodes() {
  for (int i=0; i<capacity_; i++) {
    Node<T> *curr_node = buffer_[i].load();
    Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
          util::memory::rc::unmark_first(curr_node));
    if (unmarked_curr_node->is_ElemNode()) {
      T val = unmarked_curr_node->val();
      log_dequeue[val]++;
    }
  }
  std::cout << "Lost Nodes [tid, tid&val, dequeue log, enqueue log]" << std::endl;
  std::cout << "==========================" << std::endl;
  for (auto it=log_dequeue.begin(); it!=log_dequeue.end(); ++it) {
    T key = it->first;
    if(log_dequeue[key] != 1 || log_enqueue[key] != 1) {
      int thread_id = key >> 20;
      std::cout << "[" << thread_id << "," <<  key << "," << log_dequeue[key]  <<
        "," <<   log_enqueue[key]  << "]" << std::endl;
    }
  }
}

#endif  // DEBUG

template <class T>
int64_t RingBuffer<T>::next_head_seq() {
  return head_.fetch_add(1);
}

template <class T>
int64_t RingBuffer<T>::next_tail_seq() {
  int64_t seq = tail_.fetch_add(1);
  // TODO(ATB) branch pred. expect false
  if (seq < 0) {
    // TODO(ATB) handle rollover -- after rb paper
  }
  return seq;
}

template <class T>
int64_t RingBuffer<T>::get_head_seq() {
  return head_.load();
}

template <class T>
int64_t RingBuffer<T>::get_tail_seq() {
  return tail_.load();
}

template <class T>
int64_t RingBuffer<T>::get_position(int64_t seq) {
  return seq & size_mask_;  // quickly take seq modulo capacity_ with size_mask_
}

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_RINGBUFFER_H_
