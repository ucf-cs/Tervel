#ifndef TERVEL_WFRB_DEQUEUEOP_H_
#define TERVEL_WFRB_DEQUEUEOP_H_

#include "tervel/wf-ring-buffer/wf_ring_buffer.h"
#include "tervel/wf-ring-buffer/elem_node.h"
#include "tervel/util/info.h"
#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/rc/descriptor_util.h"

#include <algorithm>
#include <atomic>
#include <cstdint>

namespace tervel {
namespace wf_ring_buffer {

template<class T>
class ElemNode;

template<class T>
class RingBuffer;

/**
 * Class used for placement in the Op Table to complete an operation that failed
 *    to complete in a bounded number of steps
 */
template<class T>
class DequeueOp : public util::OpRecord {
 public:
  explicit DequeueOp<T>(RingBuffer<T> *buffer)
      : buffer_(buffer) {}

  ~DequeueOp<T>() {}

  /**
   * This function overrides the virtual function in the OpRecord class
   * It is called by the progress assurance scheme.
   */
  void help_complete() {
    buffer_->wf_dequeue(this);
  }

  bool result(T *val) {
    if (node_.load() == FAILED) {
      return false;
    } else {
      *val = node_.load()->val();
      return true;
    }
  }

  void try_set_failed() {
    ElemNode<T> *temp = nullptr;
    node_.compare_and_exchange(temp, FAILED);
  }

 private:
  RingBuffer<T> *buffer_ { nullptr };
  std::atomic<ElemNode<T> *> node_ { nullptr };
  static constexpr ElemNode<T> *FAILED = reinterpret_cast<ElemNode<T> *>(0x1L);
};  // DequeueOp class

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_DEQUEUEOP_H_
