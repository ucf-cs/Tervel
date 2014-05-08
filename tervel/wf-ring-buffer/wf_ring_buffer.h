#ifndef TERVEL_WFRB_RINGBUFFER_H_
#define TERVEL_WFRB_RINGBUFFER_H_

#include "tervel/wf-ring-buffer/node.h"
#include "tervel/util/info.h"
#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/rc/descriptor_util.h"

#include <algorithm>
#include <cstdint>

namespace tervel {
namespace wf_ring_buffer {
/**
 * TODO class desc
 */
template<class T>
class RingBuffer : public util::memory::hp::Element {
 public:
  static constexpr T FAIL_CONST = reinterpret_cast<T>(0x1L);

  explicit RingBuffer<T>(int capacity, int num_threads)
      : capacity_(capacity)
      , num_threads_(num_threads)
  {
    size_mask_ = capacity_ - 1;
    buffer_ = new T[capacity_];
    for (int i = 0; i < capacity_; i++) {
      buffer_[i] = new Node(i);
    }
  }

  ~RingBuffer<T>() {
    for (int i = 0; i < num_threads_; i++) {
      Helper<T>* helper = helper_table_[i].helper_.load();
      // The No check flag is true because each was check prior
      // to the call of this descructor.
      if (helper == FAIL_CONST) {
        break;
      }
      util::memory::rc::free_descriptor(helper, true);
    }
  }

  /**
   * TODO: Enqueues buffer element...
   */
  bool enqueue(T *val);

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
  bool lf_enqueue();

  /**
   * TODO: Performs a dequeue...
   */
  bool lf_dequeue();

  long next_head_seq();
  long next_tail_seq();
  long get_position();

  int capacity_;
  int num_threads_;
  T *buffer_;
  long head_;
  long tail_;
};  // RingBuffer class


template<class T>
bool RingBuffer::enqueue(T* value) {
  ProgressAssurance::check_for_announcement();
  return lf_enqueue(value);
}

template<class T>
bool RingBuffer::dequeue(T *result) {
  ProgressAssurance::check_for_announcement();
  return lf_dequeue(result);
}


template<class T>
bool RingBuffer::lf_enqueue(T val) {
  int fail_count = 0;
  while (true) {
    if (is_full()) {
      return false;
    }
    long seq = next_head_seq();
    long pos = get_position(seq);
    while (true) {
      if (fail_count++ == util::ProgressAssurance::MAX_FAILURES) {
        EnqueueOp *op = new EnqueueOp(this, val);
        ProgressAssurance::make_announcement(op);
        return op->result();
      }
      Node *curr_node = buffer_[pos].load();
      Node *unmarked_curr_node = util::memory::rc::unmark_first(curr_node);
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
        if (unmarked_curr_node->seq() == seq) {
          assert(unmarked_curr_node->is_EmptyNode());
          Node *new_node = new ElemNode(seq, val);
          bool cas_success = buffer_[pos].compare_exchange_strong(
                unmarked_curr_node, new_node);
          if (cas_success) {
            util::memory::rc:free_descriptor(unmarked_curr_node);
            return true;
          } else {
            util::memory::rc:free_descriptor(new_node, true);
            break;
          }
        } else if (curr_node->seq() > seq)) {
          break;
        }
        // Otherwise, (curr_node->seq() < seq) and we must set skipped if no
        // progress occurs after backoff
        util::backoff();
        if (curr_node == buffer_[pos].load()) {  // curr_node hasnt changed
          if (curr_node->is_EmptyNode()) {
            Node *new_node = new ElemNode(seq, val);
            bool cas_success = buffer_[pos].compare_exchange_strong(
                  unmarked_curr_node, new_node);
            if (cas_success) {
              util::memory::rc:free_descriptor(unmarked_curr_node);
              return true;
            } else {  // CAS fail
              util::memory::rc:free_descriptor(new_node, true);
              break;
            }
          }  // curr_node isnt NullNode
        }  // curr_node changed during backoff
      }  // else (curr_node isnt marked skipped)
    }  // while (true)
  }  // while (true)
}  // lf_enqueue(T val)

template<class T>
bool RingBuffer::lf_dequeue(T *result) {
  int fail_count = 0;
  while (true) {
    if (is_empty()) {
      return false;
    }
    long seq = next_head_seq();
    long pos = get_position(seq);
    while (true) {
      if (fail_count++ == util::ProgressAssurance::MAX_FAILURES) {
        DequeueOp *op = new DequeueOp(this);
        ProgressAssurance::make_announcement(op);
        return op->result(result));
      }
      Node *curr_node = buffer_[pos].load();
      Node *unmarked_curr_node = util::memory::rc::unmark_first(curr_node);
      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
                                                &(buffer_[pos],
                                                curr_node));
      if (!watch_succ) {
        continue;
      }
      if (curr_node != unmarked_curr_node) {  // curr_node is marked skipped
        if (unmarked_curr_node->is_EmptyNode()) {
          Node *new_node = new NullNode(seq + capacity_);
          bool cas_success = buffer_[pos].compare_exchange_strong(curr_node,
                                                                  new_node);
          if (cas_success) {
            util::memory::rc:free_descriptor(unmarked_curr_node);
            break;
          } else {
            util::memory::rc:free_descriptor(new_node, true);
            continue;
          }
        } else {  // curr_node is ElemNode
          ElemNode *elem_node = reinterpret_cast<ElemNode*>(unmarked_curr_node);
          if (elem_node->is_owned()) {
            // We cant take an element that is already associated with another
            // thread's OpRecord
            break;
          } else {
            if (unmarked_curr_node->seq() == seq) {
              // We must mark the node as out of sync
              buffer_[pos].store(make_skipped(new_node));
              *result = unmarked_curr_node->val();
              util::memory::rc:free_descriptor(unmarked_curr_node);
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
            Node *new_node = new NullNode(seq + capacity_);
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
          util::memory::rc::atomic_mark_first(&(buffer_[pos]));
        }
      }  // else (is not skipped)
    }  // while (true)
  }  // while (true)
}  // lf_dequeue(T result)


template<class T>
void RingBuffer::wf_enqueue(EnqueueOp *op) {
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
      Node *curr_node = buffer_[pos].load();
      Node *unmarked_curr_node = util::memory::rc::unmark_first(curr_node);
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
          Node *new_node = new ElemNode(seq, val, op);
          bool cas_success = buffer_[pos].compare_exchange_strong(
                unmarked_curr_node, new_node);
          if (cas_success) {
            bool assoc_succ = op->associate();
            if (!assoc_succ) {
              Node *empty_node = new EmptyNode(seq + capacity_);
              curr_node = new_node;
              cas_success = buffer_[pos].compare_exchange_strong(
                    curr_node, empty_node);
              if (cas_success) {
                util::memory::rc:free_descriptor(new_node);
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
            util::memory::rc:free_descriptor(unmarked_curr_node);
            return;
          } else {
            util::memory::rc:free_descriptor(new_node, true);
            break;
          }
        } else if (curr_node->seq() > seq)) {
          break;
        }
        // Otherwise, (curr_node->seq() < seq) and we must set skipped if no
        // progress occurs after backoff
        util::backoff();
        if (curr_node == buffer_[pos].load()) {  // curr_node hasnt changed
          if (curr_node->is_EmptyNode()) {
            // TODO - use above abstracted code c(%*%)
          }  // curr_node isnt NullNode
        }  // curr_node changed during backoff
      }  // else (curr_node isnt marked skipped)
    }  // while (true)
  }  // while (true)
}  // wf_enqueue()

template<class T>
void RingBuffer::wf_dequeue(DequeueOp *op) {
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
      Node *curr_node = buffer_[pos].load();
      Node *unmarked_curr_node = util::memory::rc::unmark_first(curr_node);
      bool watch_succ = util::memory::rc::watch(unmarked_curr_node,
                                                &(buffer_[pos],
                                                curr_node));
      if (!watch_succ) {
        continue;
      }
      if (curr_node != unmarked_curr_node) {  // curr_node is marked skipped
        if (unmarked_curr_node->is_EmptyNode()) {
          Node *new_node = new NullNode(seq + capacity_);
          bool cas_success = buffer_[pos].compare_exchange_strong(curr_node,
                                                                  new_node);
          if (cas_success) {
            util::memory::rc:free_descriptor(unmarked_curr_node);
            break;
          } else {
            util::memory::rc:free_descriptor(new_node, true);
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
            Node *new_node = new ElemNode(seq, unmarked_curr_node->val(), op);
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

        } else if (curr_node->seq() > seq)) {
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

bool is_empty() {
  return head_ >= tail_;
}

bool is_full() {
  return tail_ >= head_+capacity_;
}

long next_head_seq() {
  return __sync_fetch_and_add(&head_, 1);
}

long next_tail_seq() {
  long seq = __sync_fetch_and_add(&tail_, 1);
  // TODO(ATB) branch pred. expect false
  if (seq < 0) {
    // TODO(ATB) handle rollover -- after rb paper
  }
  return seq;
}

long get_head_seq() {
  return head_.load();
}

long get_tail_seq() {
  return tail_.load();
}

long get_position(long seq) {
  return seq & size_mask_  // quickly take seq modulo capacity_ with size_mask_
}

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_RINGBUFFER_H_
