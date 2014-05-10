#ifndef TERVEL_WFRB_DEQUEUEOP_H_
#define TERVEL_WFRB_DEQUEUEOP_H_

#include "tervel/wf-ring-buffer/buffer_op.h"
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

/**
 * Class used for placement in the Op Table to complete an operation that failed
 *    to complete in a bounded number of steps
 */
template<class T>
class DequeueOp : public BufferOp<T> {
 public:
  explicit DequeueOp<T>(RingBuffer<T> *buffer)
      : BufferOp<T>(buffer) {}

  ~DequeueOp<T>() {}

  /**
   * This function overrides the virtual function in the OpRecord class
   * It is called by the progress assurance scheme.
   */
//  friend bool RingBuffer<T>::wf_dequeue(DequeueOp<T> *op);
  void help_complete() {
    this->buffer_->wf_dequeue(this);
  }

  bool result(T *val) {
    if (this->node_.load() != BufferOp<T>::FAILED) {
      *val = this->node_.load()->val();
      return true;
    }
    return false;
  }
};  // DequeueOp class

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_DEQUEUEOP_H_
