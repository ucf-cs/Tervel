#ifndef TERVEL_WFRB_ENQUEUEOP_H_
#define TERVEL_WFRB_ENQUEUEOP_H_

#include "tervel/wf-ring-buffer/elem_node.h"
#include "tervel/wf-ring-buffer/wf_ring_buffer.h"
#include "tervel/util/info.h"
#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/rc/descriptor_util.h"

#include <algorithm>
#include <atomic>
#include <cstdint>

namespace tervel {
namespace wf_ring_buffer {

/**
 * Class used for placement in the Op Table to complete an operation that failed
 *    to complete in a bounded number of steps
 */
template<class T>
class EnqueueOp : public util::OpRecord {
 public:

  explicit EnqueueOp<T>(RingBuffer<T> *buffer, T value)
        : buffer_(buffer)
        , value_(value) {}

  ~EnqueueOp<T>() {}

  /**
   * This function overrides the virtual function in the OpRecord class
   * It is called by the progress assurance scheme.
   */
  void help_complete() {
    buffer_->wf_enqueue(this, value_);
  }

  using util::memory::hp::Element;
  bool on_watch(std::atomic<void*> *address, void *value) {
    // TODO try to associate
    // if not associated
    //  then remove per wf_enqueue function methodology
    return false;
  }


 private:
  RingBuffer *buffer_ { nullptr };
  std::atomic<ElemNode*> node_ { nullptr };
  T value_;
  static constexpr ElemNode *FAILED = reinterpret_cast<T>(0x1L);
};  // EnqueueOp class

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_ENQUEUEOP_H_
