#ifndef TERVEL_WFRB_RINGBUFFER_H_
#define TERVEL_WFRB_RINGBUFFER_H_

#include "tervel/wf-ring-buffer/wf_ring_buffer_helper.h"
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
  static constexpr T MCAS_FAIL_CONST = reinterpret_cast<T>(0x1L); // what is
                                                                  // this and do
                                                                  // I need it?

  explicit RingBuffer<T>(int capacity, int num_threads)
      : capacity_(capacity)
      , num_threads_(num_threads) {}

  ~RingBuffer<T>() {
    for (int i = 0; i < num_threads_; i++) {
      Helper<T>* helper = helper_table_[i].helper_.load();
      // The No check flag is true because each was check prior
      // to the call of this descructor.
      if (helper == MCAS_FAIL_CONST) {
        break;
      }
      util::memory::rc::free_descriptor(helper, true);
    }
  }

  /**
   * TODO: Initializes buffer...
   */
  bool init();

  /**
   * TODO: Enqueues buffer element...
   */
  bool enqueue();

  /**
   * TODO: Dequeues buffer element...
   */
  bool dequeue();

  /**
   * @return whether the buffer is empty
   */
  bool isEmpty();

  /**
   * @return whether the buffer is full
   */
  bool isFull();

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
RingBuffer::init() {
  buffer_ = new T[capacity_];
  for (int i = 0; i < capacity_; i++) {
    buffer_[i] = new Node(i);
  }
}

template<class T>
bool RingBuffer::enqueue() {
  ProgressAssurance::enqueuer_check_for_announcement();
}

template<class T>
bool RingBuffer::dequeue() {
  ProgressAssurance::dequeuer_check_for_announcement();
}


template<class T>
bool RingBuffer::lf_enqueue() {

}

template<class T>
bool RingBuffer::lf_dequeue(T result) {
  int fail_count = 0;
  while (true) {
    if (is_empty()) {
      return false;
    }
    long seq = next_head_seq();
    long pos = get_position(seq);
    while (true) {
      if (fail_count++ == util::ProgressAssurance::MAX_FAILURES) {
        DequeueOp *op = new DequeueOp();
        ProgressAssurance::p_make_announcement(op, tid);  // TODO tid isnt
                                                          // defined
        if (op->didSucceed()) {
          result = op->element;
          return true;
        } else {
          return false;  // @reviewer: should this be an assertion? if progress
                         // guarantee fails we break wf property and !happen
        }
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
        if (unmarked_curr_node->is_NullNode()) {
          Node *new_node = new NullNode(seq + capacity_);
          bool cas_success = buffer_[pos].compare_exchange_strong(curr_node,
                                                                  new_node);
          if (cas_success) {
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
              result = curr_node;
              util::memory::rc:free_descriptor(unmarked_current_node, true);
              return true;
            } else {
              break;
            }
          }
        }
      } else {
        if (curr_node->seq() == seq) {
          if (isElementNode(curr_node)) {
            Node *new_node = new NullNode(seq + capacity_);
            bool cas_succ = buffer_[pos].compare_exchange_strong(curr_node,
                                                                 new_node);
            if (cas_succ) {
              result = curr_node->val();
              util::memory::rc::free_descriptor(curr_node);
              return true;
            } else { // cas failed
              buffer_[pos].store(markNode(new_node));
              result = curr_node;
              // see pseudo-code "return current.element"
            }
          }

        } else if (curr_node->seq() > seq)) {
          break;
        }
        // otherwise, (curr_node->seq() < seq) and we must set skipped if no
        // progress occurs after backoff
        BackOff();
        if (curr_node == buffer_[pos].load()) {
          buffer_[pos].store(makeSkipped(buffer_[pos]));
        }
        continue;  // unnecessary continue adopted from pseudo-code
      }
    }
  }
}

long next_head_seq() {
  return __sync_fetch_and_add(&head, 1);
}

long next_tail_seq() {
  long seq = __sync_fetch_and_add(&tail, 1);
  // TODO(ATB) branch pred. expect false
  if (seq < 0) {
    // TODO(ATB) handle rollover -- after rb paper
  }
  return seq;
}

long get_position(long seq) {
  return seq & size_mask_  // quickly take seq modulo capacity_ with size_mask_
}

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_RINGBUFFER_H_
