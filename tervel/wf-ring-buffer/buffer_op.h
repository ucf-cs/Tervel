#ifndef TERVEL_WFRB_BUFFEROP_H
#define TERVEL_WFRB_BUFFEROP_H


#include "tervel/util/progress_assurance.h"
#include "tervel/wf-ring-buffer/elem_node.h"
#include "tervel/wf-ring-buffer/dequeue_op.h"
#include "tervel/wf-ring-buffer/enqueue_op.h"
#include "tervel/wf-ring-buffer/wf_ring_buffer.h"

#include <atomic>

namespace tervel {
namespace wf_ring_buffer {


template<class T>
class RingBuffer;
template<class T>
class ElemNode;
template<class T>
class EnqueueOp;
template<class T>
class DequeueOp;
/**
 * // REVIEW(steven) missing descriptionn of class
 */
template <class T>
class BufferOp : public util::OpRecord {
 public:
  explicit BufferOp<T>(RingBuffer<T> *buffer)
      : buffer_(buffer) {}

  ~BufferOp<T>() {}

  // REVIEW(steven) missing description
  void try_set_failed() {
    ElemNode<T> *null_node = nullptr;
    this->helper_.compare_exchange_strong(null_node, FAILED);
  }

  // REVIEW(steven) missing description
  /**
   * [associate description]
   * @param  node    [description]
   * @param  address [description]
   * @return whether or not this function changed the value at the address
   */
  virtual bool associate(ElemNode<T> *node, std::atomic<Node<T>*> *address) = 0;

  // REVIEW(steven) missing description
  virtual bool result() {
    assert(helper_.load());
    return helper_.load() == FAILED;
  }

  // REVIEW(steven) missing description
  virtual void help_complete() {
    assert(false);
  };

 protected:
  RingBuffer<T> *buffer_;
  std::atomic<ElemNode<T> *> helper_ {nullptr};
  static constexpr ElemNode<T> *FAILED = reinterpret_cast<ElemNode<T> *>(0x1L);

  friend class RingBuffer<T>;
  friend class DequeueOp<T>;
  friend class ElemNode<T>;
  friend class EnqueueOp<T>;
};  // BufferOp class

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_BUFFEROP_H
