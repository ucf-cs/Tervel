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
 *
 */
template <class T>
class BufferOp : public util::OpRecord {
 public:
  explicit BufferOp<T>(RingBuffer<T> *buffer)
      : buffer_(buffer){}

  ~BufferOp<T>() {}

  void try_set_failed() {
    associate(FAILED);
  }

  bool associate(ElemNode<T> * node) {
    ElemNode<T> * temp = node_.load();
    if (temp == nullptr) {
      bool succ = node_.compare_exchange_strong(temp, node);
      return succ;
    }
    return false;
  }

  virtual bool result() {
    return node_.load() == FAILED;
  }
  virtual void help_complete() {
    assert(false);
  };

 protected:
  RingBuffer<T> *buffer_;
  std::atomic<ElemNode<T> *> node_ {nullptr};
  static constexpr ElemNode<T> *FAILED = reinterpret_cast<ElemNode<T> *>(0x1L);

  friend class RingBuffer<T>;
  friend class DequeueOp<T>;
  friend class ElemNode<T>;
  friend class EnqueueOp<T>;
};  // BufferOp class

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_BUFFEROP_H
