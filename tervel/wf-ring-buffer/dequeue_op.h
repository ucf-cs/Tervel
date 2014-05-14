#ifndef TERVEL_WFRB_DEQUEUEOP_H_
#define TERVEL_WFRB_DEQUEUEOP_H_

#include "tervel/wf-ring-buffer/buffer_op.h"
#include "tervel/wf-ring-buffer/node.h"
#include "tervel/wf-ring-buffer/elem_node.h"
#include "tervel/wf-ring-buffer/wf_ring_buffer.h"

#include <algorithm>
#include <atomic>
#include <cstdint>

namespace tervel {
namespace wf_ring_buffer {


template<class T>
class RingBuffer;

template<class T>
class BufferOp;

template<class T>
class Node;

template<class T>
class ElemNode;
/**
 * Class used for placement in the Op Table to complete an operation that failed
 *    to complete in a bounded number of steps
 */
template<class T>
class DequeueOp : public BufferOp<T> {
 public:
  explicit DequeueOp<T>(RingBuffer<T> *buffer)
      : BufferOp<T>(buffer) {}

  ~DequeueOp<T>() {
    ElemNode<T> *temp = this->helper_.load();
    assert(temp);
    if (temp != BufferOp<T>::FAILED) {
      util::memory::rc::free_descriptor(temp, true);
    }
  }

  bool on_is_watched() {
    // Not should only be called by reclaimation scheme after this op has been
    // completed
    ElemNode<T> *temp = this->helper_.load();
    assert(temp);
    if (temp != BufferOp<T>::FAILED) {
      return util::memory::rc::is_watched(temp);
    }
    return false;
  }

  /**
   * This function overrides the virtual function in the OpRecord class
   * It is called by the progress assurance scheme.
   */
  void help_complete() {
    this->buffer_->wf_dequeue(this);
  }

  // Review(steven): missing description
  bool associate(ElemNode<T> *node, std::atomic<Node<T>*> *address) {
    ElemNode<T> *null_node = nullptr;
    bool success = this->helper_.compare_exchange_strong(null_node, node);
    if (success || null_node == node) {
      Node<T> *curr_node = reinterpret_cast<Node<T> *>(node);

      Node<T> *new_node = reinterpret_cast<Node<T> *>(
          util::memory::rc::get_descriptor< EmptyNode<T> >(curr_node->seq() +
          this->buffer_->capacity()));

      success = address->compare_exchange_strong(curr_node, new_node);
      if (!success) {  // node may have been marked as skipped
        curr_node = reinterpret_cast<Node<T> *>(
                    tervel::util::memory::rc::mark_first(curr_node));

        if (address->load() == curr_node) {
          Node<T> *marked_node = reinterpret_cast<Node<T> *>(
                    tervel::util::memory::rc::mark_first(new_node));

          success = address->compare_exchange_strong(curr_node, marked_node);
          if (!success) {
            util::memory::rc::free_descriptor(new_node, true);
          }
        }
      }
      return true;
    } else {
      node->clear_op();
      return false;
    }
  }

  // REVIEW(steven) missing description
  bool result(T *val) {
    if (this->helper_.load() != BufferOp<T>::FAILED) {
      *val = this->helper_.load()->val();
      return true;
    }
    return false;
  }
};  // DequeueOp class

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_DEQUEUEOP_H_
