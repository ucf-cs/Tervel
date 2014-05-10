#ifndef TERVEL_WFRB_RINGBUFFER_H_
#define TERVEL_WFRB_RINGBUFFER_H_

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

namespace tervel {
namespace wf_ring_buffer {

template<class T>
class EnqueueOp;
template<class T>
class DequeueOp;
template<class T>
class NodeOp;
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
    size_mask_ = capacity_ - 1;
    buffer_ = new std::atomic<Node<T> *>[capacity_];
    for (long i = 0; i < capacity_; i++) {
      Node<T> *empty_node = reinterpret_cast<Node<T> *>(util::memory::rc::get_descriptor<EmptyNode<T>>(i));
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

private:
  /**
   * TODO: Performs an enqueue..
   */
  bool lf_enqueue(T value);

  /**
   * TODO: Performs a dequeue...
   */
  bool lf_dequeue(T *result);

  bool wf_enqueue(EnqueueOp<T> *op);

  bool wf_dequeue(DequeueOp<T> *op);

  long next_head_seq();
  long next_tail_seq();
  long get_head_seq();
  long get_tail_seq();
  long get_position(long seq);

  int capacity_;
  int size_mask_;
  std::atomic<Node<T> *> *buffer_;
  std::atomic<long> head_;
  std::atomic<long> tail_;
};  // RingBuffer class


template<class T>
bool RingBuffer<T>::enqueue(T value) {
  util::ProgressAssurance::check_for_announcement();
  return lf_enqueue(value);
}

template<class T>
bool RingBuffer<T>::dequeue(T *result) {
  util::ProgressAssurance::check_for_announcement();
  return lf_dequeue(result);
}


template<class T>
bool RingBuffer<T>::lf_enqueue(T val) {
  unsigned int fail_count = 0;
  while (true) {
    if (is_full()) {
      return false;
    }
    long seq = next_head_seq();
    long pos = get_position(seq);
    while (true) {
      if (fail_count++ == util::ProgressAssurance::MAX_FAILURES) {
        EnqueueOp<T> *op = new EnqueueOp<T>(this, val);
        util::ProgressAssurance::make_announcement(op);
        return op->result();
      }
      Node<T> *curr_node = buffer_[pos].load();
      Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(util::memory::rc::unmark_first(curr_node));
      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
                                                reinterpret_cast<std::atomic<void *> *>(&(buffer_[pos])),
                                                curr_node);
      if (!watch_succ) {
        continue;
      }
      if (curr_node != unmarked_curr_node) {  // curr_node is marked skipped
        // Enqueues do not modify marked nodes
        break;
      } else {  // curr_node isnt marked skipped
        if (unmarked_curr_node->seq() == seq) {
          assert(unmarked_curr_node->is_EmptyNode());
          Node<T> *new_node = reinterpret_cast<Node<T> *>(util::memory::rc::get_descriptor<ElemNode<T>>(seq, val));
          bool cas_success = buffer_[pos].compare_exchange_strong(
                unmarked_curr_node, new_node);
          if (cas_success) {
            util::memory::rc::free_descriptor(unmarked_curr_node);
            return true;
          } else {
            util::memory::rc::free_descriptor(new_node, true);
            break;
          }
        } else if (curr_node->seq() > seq) {
          break;
        }
        // Otherwise, (curr_node->seq() < seq) and we must set skipped if no
        // progress occurs after backoff
        util::backoff();
        if (curr_node == buffer_[pos].load()) {  // curr_node hasnt changed
          if (curr_node->is_EmptyNode()) {
            Node<T> *new_node = reinterpret_cast<Node<T> *>(util::memory::rc::get_descriptor<ElemNode<T>>(seq, val));
            bool cas_success = buffer_[pos].compare_exchange_strong(
                  unmarked_curr_node, new_node);
            if (cas_success) {
              util::memory::rc::free_descriptor(unmarked_curr_node);
              return true;
            } else {  // CAS fail
              util::memory::rc::free_descriptor(new_node, true);
              break;
            }
          }  // curr_node isnt NullNode
        }  // curr_node changed during backoff
      }  // else (curr_node isnt marked skipped)
    }  // while (true)
  }  // while (true)
}  // lf_enqueue(T val)

template<class T>
bool RingBuffer<T>::lf_dequeue(T *result) {
  unsigned int fail_count = 0;
  while (true) {
    if (is_empty()) {
      return false;
    }
    long seq = next_head_seq();
    long pos = get_position(seq);
    while (true) {
      if (fail_count++ == util::ProgressAssurance::MAX_FAILURES) {
        DequeueOp<T> *op = new DequeueOp<T>(this);
        util::ProgressAssurance::make_announcement(op);
        return op->result(result);
      }
      Node<T> *curr_node = buffer_[pos].load();
      Node<T> *unmarked_curr_node = reinterpret_cast<Node<T> *>(util::memory::rc::unmark_first(curr_node));
      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
                                                &(buffer_[pos],
                                                curr_node));
      if (!watch_succ) {
        continue;
      }
      if (curr_node != unmarked_curr_node) {  // curr_node is marked skipped
        if (unmarked_curr_node->is_EmptyNode()) {
          Node<T> *new_node = reinterpret_cast<Node<T> *>(util::memory::rc::get_descriptor<EmptyNode<T>>(seq + capacity_));
          bool cas_success = buffer_[pos].compare_exchange_strong(curr_node,
                                                                  new_node);
          if (cas_success) {
            util::memory::rc::free_descriptor(unmarked_curr_node);
            break;
          } else {
            util::memory::rc::free_descriptor(new_node, true);
            continue;
          }
        } else {  // curr_node is ElemNode
          ElemNode<T> *elem_node = reinterpret_cast<ElemNode<T>*>(unmarked_curr_node);
          if (elem_node->is_owned()) {
            // We cant take an element that is already associated with another
            // thread's OpRecord
            break;
          } else {
            if (unmarked_curr_node->seq() == seq) {
              // We must mark the node as out of sync
              Node<T> *new_node = reinterpret_cast<Node<T> *>(util::memory::rc::get_descriptor<EmptyNode<T>>(seq + capacity_));
              buffer_[pos].store(make_skipped(new_node));
              *result = unmarked_curr_node->val();
              util::memory::rc::free_descriptor(unmarked_curr_node);
              return true;
            } else {
              break;
            }
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
            Node<T> *new_node = reinterpret_cast<Node<T> *>(util::memory::rc::get_descriptor<EmptyNode<T>>(seq + capacity_));
            bool cas_succ = buffer_[pos].compare_exchange_strong(
                  unmarked_curr_node, new_node);
            if (cas_succ) {
              *result = curr_node->val();
              util::memory::rc::free_descriptor(unmarked_curr_node);
              return true;
            } else {
              // The value may have been bitmarked or been replaced by a copy
              // with an OpRecord
              continue;
            }
          }

        } else if (curr_node->seq() > seq) {
          break;
        }
        // Otherwise, (curr_node->seq() < seq) and we must set skipped if no
        // progress occurs after backoff
        util::backoff();
        if (curr_node == buffer_[pos].load()) {
          util::memory::rc::atomic_mark_first(reinterpret_cast<std::atomic<void*> *>(&(buffer_[pos])));
        }
      }  // else (is not skipped)
    }  // while (true)
  }  // while (true)
}  // lf_dequeue(T result)

template<class T>
bool RingBuffer<T>::wf_enqueue(EnqueueOp<T> *op) {
  while (true) {
    if (is_full()) {
      op->try_set_failed();
      return;
    }
    long seq = next_tail_seq();
    long pos = get_position(seq);
    while (true) {
      if (op->helper_.load() != nullptr) {
        return;
      }
      Node<T> *curr_node = buffer_[pos].load();
      Node<T> *unmarked_curr_node = util::memory::rc::unmark_first(curr_node);
      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
                                                &(buffer_[pos],
                                                curr_node));
      if (!watch_succ) {
        continue;
      }
      if (curr_node != unmarked_curr_node) {  // curr_node is marked skipped
        // Enqueues do not modify marked nodes
        break;
      } else {  // curr_node isnt marked skipped
        if (unmarked_curr_node->seq() == seq) {  // abstract contents to a
                                                 // function c(%*%)
          assert(unmarked_curr_node->is_EmptyNode());
          Node<T> *new_node = reinterpret_cast<Node<T> *>(util::memory::rc::get_descriptor<ElemNode<T>>(seq, op->val, op));
          bool cas_success = buffer_[pos].compare_exchange_strong(
                unmarked_curr_node, new_node);
          if (cas_success) {
            bool assoc_succ = op->associate();
            if (!assoc_succ) {
              Node<T> *empty_node = reinterpret_cast<Node<T> *>(util::memory::rc::get_descriptor<EmptyNode<T>>(seq + capacity_));
              curr_node = new_node;
              cas_success = buffer_[pos].compare_exchange_strong(
                    curr_node, empty_node);
              if (cas_success) {
                util::memory::rc::free_descriptor(new_node);
              } else if (curr_node == util::memory::rc::mark_first(new_node)) {
                // CAS failed due to a bitmark
                cas_success = buffer_[pos].compare_exchange_strong(
                    curr_node, util::memory::rc::mark_first(empty_node));
                if (cas_success) {
                  util::memory::rc::free_descriptor(new_node);
                } else {
                  util::memory::rc::free_descriptor(empty_node, true);
                }
              }
            }
            util::memory::rc::free_descriptor(unmarked_curr_node);
            return;
          } else {
            util::memory::rc::free_descriptor(new_node, true);
            break;
          }
        } else if (curr_node->seq() > seq) {
          break;
        }
        // Otherwise, (curr_node->seq() < seq) and we must set skipped if no
        // progress occurs after backoff
        util::backoff();
        if (curr_node == buffer_[pos].load()) {  // curr_node hasnt changed
          if (curr_node->is_EmptyNode()) {
            // TODO - use above abstracted code c(%*%)
          }  // curr_node isnt EmptyNode
        }  // curr_node changed during backoff
      }  // else (curr_node isnt marked skipped)
    }  // while (true)
  }  // while (true)
}  // wf_enqueue()

template<class T>
bool RingBuffer<T>::wf_dequeue(DequeueOp<T> *op) {
  long seq = get_head_seq()-1;
  while (true) {
    if (is_empty()) {
      op->try_set_failed();
      return;
    }
    seq++;
    long pos = get_position(seq);
    while (true) {
      if (op->node_.load() != nullptr) {
        return;
      }
      Node<T> *curr_node = buffer_[pos].load();
      Node<T> *unmarked_curr_node = util::memory::rc::unmark_first(curr_node);
      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
                                                &(buffer_[pos],
                                                curr_node));
      if (!watch_succ) {
        continue;
      }
      if (curr_node != unmarked_curr_node) {  // curr_node is marked skipped
        if (unmarked_curr_node->is_EmptyNode()) {
          Node<T> *new_node = reinterpret_cast<Node<T> *>(util::memory::rc::get_descriptor<EmptyNode<T>>(seq + capacity_));
          bool cas_success = buffer_[pos].compare_exchange_strong(curr_node,
                                                                  new_node);
          if (cas_success) {
            util::memory::rc::free_descriptor(unmarked_curr_node);
            break;
          } else {
            util::memory::rc::free_descriptor(new_node, true);
            continue;
          }
        } else {  // curr_node is ElemNode
          // curr_node is a skipped ElemNode. It must be removed by a thread
          // with the values seq number so it may be corrected.
          break;
        }
      } else {  // curr_node isnt marked skipped
        if (unmarked_curr_node->seq() == seq) {
          if (unmarked_curr_node->is_ElemNode()) {
            // The current node is an ElemNode and we are using its seq
            // number. We also are sure it is not associated with an OpRecord.
            Node<T> *new_node = reinterpret_cast<Node<T> *>(util::memory::rc::get_descriptor<ElemNode<T>>(seq, unmarked_curr_node->val(), op));
            // ..Since no other thread can remove the value, we CAS to be able
            // to determine if the position has been bitmarked..
            bool cas_succ = buffer_[pos].compare_exchange_strong(
                  unmarked_curr_node, new_node);
            if (cas_succ) {
              util::memory::rc::free_descriptor(unmarked_curr_node);
              new_node->associate();
              return;
            } else {
              buffer_[pos].store(util::memory::rc::mark_first(new_node));
            }
          }

        } else if (curr_node->seq() > seq) {
          break;
        }
        // Otherwise, (curr_node->seq() < seq) and we must set skipped if no
        // progress occurs after backoff
        util::backoff();
        if (curr_node == buffer_[pos].load()) {
          util::memory::rc::atomic_mark_first(&(buffer_[pos]));
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
