#ifndef TERVEL_WFRB_RINGBUFFER_H_
#define TERVEL_WFRB_RINGBUFFER_H_

#ifdef DEBUG
  #include "tbb/concurrent_hash_map.h"
#endif

#include "tervel/wf-ring-buffer/node.h"
#include "tervel/wf-ring-buffer/empty_node.h"
#include "tervel/wf-ring-buffer/elem_node.h"
#include "tervel/wf-ring-buffer/enqueue_op.h"
#include "tervel/wf-ring-buffer/dequeue_op.h"
#include "tervel/util/info.h"
#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/rc/descriptor_util.h"

#include <atomic>
#include <algorithm>
#include <cstdint>
#include <map>
#include <mutex>

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

  explicit RingBuffer<T>(int capacity)
      : capacity_(capacity)
  {
    // REVIEW(steven) need min capacity check
    size_mask_ = capacity_ - 1;

    buffer_ = new std::atomic<Node<T> *>[capacity_];
    // REVIEW(steven)
    for (long i = 0; i < capacity_; i++) {
      // REVIEW(steven) line to long
      Node<T> *empty_node = util::memory::rc::get_descriptor<EmptyNode<T>>(i);
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
  bool dequeue(T *result);

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
  bool content_is_full() __attribute__((used));

  bool content_is_empty() __attribute__((used));

  void print_invalid_nodes() __attribute__((used));

  void print_lost_nodes() __attribute__((used));

  #endif  // DEBUG

  int capacity()  {return capacity_;} // REVIEW(steven) could be a const
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
  long next_head_seq();

  // REVIEW(steven) missing description
  long next_tail_seq();

  // REVIEW(steven) missing description
  long get_head_seq();

  // REVIEW(steven) missing description
  long get_tail_seq();

  // REVIEW(steven) missing description
  long get_position(long seq);

  int capacity_;  // REVIEW(steven) could be a const
  int size_mask_; // if done in the initilization list
  std::atomic<Node<T> *> *buffer_;
  std::atomic<long> head_ {0}; // REVIEW(steven) should be initlized
  std::atomic<long> tail_ {0}; // REVIEW(steven) should be initilized

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
  if (succ) {
    log_mutex_enqueue.lock();
    int temp = log_enqueue[value];
    assert(temp == 0);
    log_enqueue[value]++;
    log_mutex_enqueue.unlock();
  }
  return succ;
}

template<class T>
bool RingBuffer<T>::dequeue(T *result) {
  util::ProgressAssurance::check_for_announcement();
  bool succ = lf_dequeue(result);
  if (succ) {
    log_mutex_dequeue.lock();
    int temp = log_dequeue[*result];
    assert(temp == 0);
    log_dequeue[*result]++;
    log_mutex_dequeue.unlock();
  }
  return succ;
}

template<class T>
bool RingBuffer<T>::lf_enqueue(T val) {
  unsigned int fail_count = 0;
  while (true) { // REVIEW(steven) describe loop
    if (is_full()) {
      return false;
    }

    long seq = next_tail_seq();
    long pos = get_position(seq);

    // REVIEW(steven) describe loop
    while (true) {
      if (fail_count++ == util::ProgressAssurance::MAX_FAILURES) {
        EnqueueOp<T> *op = new EnqueueOp<T>(this, val);
        util::ProgressAssurance::make_announcement(
              reinterpret_cast<tervel::util::OpRecord *>(op));
        bool result = op->result();
        op->safe_delete();
        return result;
      }

      Node<T> *curr_node = buffer_[pos].load();
      Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
            util::memory::rc::unmark_first(curr_node));
      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
            reinterpret_cast<std::atomic<void *> *>(&(buffer_[pos])),
            curr_node);

      if (!watch_succ) {
        continue;
      }
      else if (curr_node != unmarked_curr_node) {  // curr_node is skipped marked
        // Enqueues do not modify marked nodes
        break;
      } else {  // curr_node isnt marked skipped
        if (curr_node->seq() < seq) {
            util::backoff();
            if (curr_node != buffer_[pos].load()) {  // curr_node has changed
              continue;  // reprocess the current value.
            }
        }
        if (curr_node->seq() <= seq && curr_node->is_EmptyNode()) {
          Node<T> *new_node = util::memory::rc::get_descriptor< ElemNode<T> >(val, seq);
          bool cas_success = buffer_[pos].compare_exchange_strong(
                curr_node, new_node);

          if (cas_success) {
            util::memory::rc::free_descriptor(curr_node);
            return true;
          } else {
            util::memory::rc::free_descriptor(new_node, true);
            break;
          }
        } else {  // (curr_node->seq() > seq) {
          break;
        }
      }  // else (curr_node isnt marked skipped)
    }  // while (true)
  }  // while (true)
}  // lf_enqueue(T val)

template<class T>
bool RingBuffer<T>::lf_dequeue(T *result) {
  unsigned int fail_count = 0;
  while (true) { // REVIEW(steven) describe loop
    if (is_empty()) {
      return false;
    }

    long seq = next_head_seq();
    long pos = get_position(seq);
    while (true) { // REVIEW(steven) describe loop
      if (fail_count++ == util::ProgressAssurance::MAX_FAILURES) {

        DequeueOp<T> *op = new DequeueOp<T>(this);
        util::ProgressAssurance::make_announcement(
              reinterpret_cast<tervel::util::OpRecord *>(op));
        bool op_succ = op->result(result);
        op->safe_delete();
        return op_succ;
      }

      Node<T> *curr_node = buffer_[pos].load();
      Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
            util::memory::rc::unmark_first(curr_node));

      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
            reinterpret_cast<std::atomic<void*> *>(&(buffer_[pos])), curr_node);

      if (!watch_succ) {
        continue;
      }
      if (curr_node != unmarked_curr_node) {  // curr_node is marked skipped
        if (unmarked_curr_node->is_EmptyNode()) {
          Node<T> *new_node = util::memory::rc::get_descriptor<EmptyNode<T>>(seq + capacity_);

          bool cas_success = buffer_[pos].compare_exchange_strong(curr_node,
                                                                  new_node);
          if (cas_success) {
            util::memory::rc::free_descriptor(unmarked_curr_node);
            break;
          } else {
            util::memory::rc::free_descriptor(new_node, true);
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
              util::memory::rc::free_descriptor(unmarked_curr_node);
              *result = unmarked_curr_node->val();
              return true;
            } else {
              util::memory::rc::free_descriptor(new_node, true);
              continue;
            }
          } else {
            // REVIEW(steven): explain this break statement
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
                  unmarked_curr_node, new_node);

            if (cas_succ) {
              *result = curr_node->val();
              util::memory::rc::free_descriptor(unmarked_curr_node);
              return true;
            } else {
              // The value may have been bitmarked or been replaced by a copy
              // with an OpRecord
              util::memory::rc::free_descriptor(new_node, true);
              continue;
            }
          }  // if is elemnode

        } else if (curr_node->seq() > seq) {
          break;
        } else {
          // Otherwise, (curr_node->seq() < seq) and we must set skipped if no
          // progress occurs after backoff
          util::backoff();
          if (curr_node == buffer_[pos].load()) {
            util::memory::rc::atomic_mark_first(
                  reinterpret_cast<std::atomic<void*> *>(&(buffer_[pos])));
          }
        }  // else curr_node->seq() < seq
      }  // else (is not skipped)
    }  // while (true)
  }  // while (true)
}  // lf_dequeue(T result)

template<class T>
void RingBuffer<T>::wf_enqueue(EnqueueOp<T> *op) {
  long seq = get_tail_seq() - 1;

  while (true) {
    if (is_full()) {
      op->try_set_failed();
      assert(op->helper_.load() != nullptr);
      return;
    }

    seq++;
    long pos = get_position(seq);

    while (op->helper_.load() == nullptr) {
      Node<T> *curr_node = buffer_[pos].load();
      Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
            util::memory::rc::unmark_first(curr_node));

      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
            reinterpret_cast<std::atomic<void*> *>(&(buffer_[pos])),
            curr_node);

      if (!watch_succ) {
        continue;
      }
      if (curr_node != unmarked_curr_node) {  // curr_node is marked skipped
        // Enqueues do not modify marked nodes
        break;
      } else {  // curr_node isnt marked skipped
        if (unmarked_curr_node->seq() < seq) {
          util::backoff();
          if (curr_node != buffer_[pos].load()) {  // curr_node has changed
            continue;  // re-process the current value
          }  // curr_node changed during backoff
        }

        if (curr_node->seq() <= seq && curr_node->is_EmptyNode()) {
          ElemNode<T> *new_node = util::memory::rc::get_descriptor<ElemNode<T>>(
                op->value(), seq,  op);

          bool cas_success = buffer_[pos].compare_exchange_strong(curr_node, new_node);

          if (cas_success) {
            util::memory::rc::free_descriptor(curr_node);
            op->associate(new_node, &(buffer_[pos]));
            assert(op->helper_.load() != nullptr);
            return;
          } else {  // failed to place the new node
            util::memory::rc::free_descriptor(new_node, true);
          }
        } else {  // (curr_node->seq() > seq)
          break;  // break inner loop and get a new sequence number
        }
      }  // else (curr_node isnt marked skipped)
    }  // while (true)
  }  // while (true)
}  // wf_enqueue()

template<class T>
void RingBuffer<T>::wf_dequeue(DequeueOp<T> *op) {
  long seq = get_head_seq() - 1;

  while (true) {
    if (is_empty()) {
      op->try_set_failed();
      assert(op->helper_.load() != nullptr);
      return;
    }

    seq++;
    long pos = get_position(seq);

    while (op->helper_.load() == nullptr) {
      Node<T> *curr_node = buffer_[pos].load();
      Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
            util::memory::rc::unmark_first(curr_node));

      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
            reinterpret_cast<std::atomic<void *> *>(&(buffer_[pos])),
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
            break;
        } else {  // curr_node is ElemNode
          // curr_node is a skipped ElemNode. It must be removed by a thread
          // with the values seq number so it may be corrected.
          break;
        }
      } else {  // curr_node isnt marked skipped
        if (unmarked_curr_node->seq() == seq) {
          if (unmarked_curr_node->is_ElemNode()) {

            Node<T> *new_node = util::memory::rc::get_descriptor<
                ElemNode<T> >(unmarked_curr_node->val(), seq, op);

            bool cas_succ = buffer_[pos].compare_exchange_strong(
                  unmarked_curr_node, new_node);
            if (cas_succ) {
              util::memory::rc::free_descriptor(unmarked_curr_node);
              op->associate(reinterpret_cast<ElemNode<T> *>(new_node), &(buffer_[pos]));
              return;
            } else {
              util::memory::rc::free_descriptor(new_node, true);
              continue;
            }
          }

        } else if (curr_node->seq() > seq) {
          break;
        }
        // Otherwise, (curr_node->seq() < seq) and do not set skipped if no
        // progress occurs after backoff because we were not assigned a sequence
        // number
        util::backoff();
        if (curr_node == buffer_[pos].load()) {
          break;
        }
      }  // else (is not skipped)
    }  // while (true)
  }  // while (true)
}  // wf_dequeue(DequeueOp *op)

template <class T>
bool RingBuffer<T>::is_empty() {
  return head_ >= tail_;
}

template <class T>
bool RingBuffer<T>::is_full() {
  return tail_ >= head_+capacity_;
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
bool RingBuffer<T>::content_is_empty() {
  int num_elem = 0, num_empty = 0;
  for (int i=0; i<capacity_; i++) {
    Node<T> *curr_node = buffer_[i].load();
    Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
          util::memory::rc::unmark_first(curr_node));
    if (unmarked_curr_node->is_ElemNode()) {
      num_elem++;
    } else {
      num_empty++;
    }
  }
  printf("%s\n", (num_elem==0) ? "True" : "False");
  printf("ElemNode count:\t%d\nEmptyNode count:\t%d\n", num_elem, num_empty);
  return (num_elem == 0);
}

template <class T>
void RingBuffer<T>::print_invalid_nodes() {
  std::cout << "Invalid Nodes" << std::endl;
  std::cout << "==============" << std::endl;
  for (int i=0; i<capacity_; i++) {
    Node<T> *curr_node = buffer_[i].load();
    Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(
          util::memory::rc::unmark_first(curr_node));
    if (get_position(unmarked_curr_node->seq()) != i) {
      if (unmarked_curr_node->is_ElemNode()) {
        printf("Node is ElemNode:\tTrue\n");
        printf("Val:\t%ld\n", (reinterpret_cast<ElemNode<T> *>(
              unmarked_curr_node))->val());
      } else {
        printf("Node is ElemNode:\tFalse\n");
      }
      printf("Seq:\t%ld\nPos:\t%d\n", curr_node->seq(), i);
    }
  }
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
long RingBuffer<T>::next_head_seq() {
  return head_.fetch_add(1);
}

template <class T>
long RingBuffer<T>::next_tail_seq() {
  long seq = tail_.fetch_add(1);
  // TODO(ATB) branch pred. expect false
  if (seq < 0) {
    // TODO(ATB) handle rollover -- after rb paper
  }
  return seq;
}

template <class T>
long RingBuffer<T>::get_head_seq() {
  return head_.load();
}

template <class T>
long RingBuffer<T>::get_tail_seq() {
  return tail_.load();
}

template <class T>
long RingBuffer<T>::get_position(long seq) {
  return seq & size_mask_;  // quickly take seq modulo capacity_ with size_mask_
}

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_RINGBUFFER_H_
