#ifndef TERVEL_WFRB_ENQUEUEOP_H_
#define TERVEL_WFRB_ENQUEUEOP_H_

#include <tervel/containers/wf/ring-buffer/buffer_op.h>
#include <tervel/containers/wf/ring-buffer/node.h>
#include <tervel/containers/wf/ring-buffer/elem_node.h>
#include <tervel/containers/wf/ring-buffer/wf_ring_buffer.h>
#include <tervel/util/memory/rc/descriptor_util.h>

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
class EnqueueOp : public BufferOp<T> {
 public:
  explicit EnqueueOp<T>(RingBuffer<T> *buffer, T value)
        : BufferOp<T>(buffer)
        , val_(value) {}

  ~EnqueueOp<T>() {}

  /**
   * This function overrides the virtual function in the OpRecord class
   * It is called by the progress assurance scheme.
   */
  void help_complete() {
    this->buffer_->wf_enqueue(this);
  }

  // REVIEW(steven): missing description
  bool associate(ElemNode<T> *node, std::atomic<Node<T>*> *address) {
    ElemNode<T> *null_node = nullptr;
    bool success = this->helper_.compare_exchange_strong(null_node, node);
    if (success || null_node == node) {
      node->clear_op();
      return false;
    } else {
      Node<T> *curr_node = reinterpret_cast<Node<T> *>(node);

      Node<T> *new_node = reinterpret_cast<Node<T> *>(
            util::memory::rc::get_descriptor< EmptyNode<T> >(node->seq()));


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
      if (success) {
        util::memory::rc::free_descriptor(node, false);
      }
    }
    return true;
  }

  // REVIEW(steven) missing description
  T value() { return val_; }

 private:
  T val_;
};  // EnqueueOp class

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_ENQUEUEOP_H_
