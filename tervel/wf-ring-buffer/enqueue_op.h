#ifndef TERVEL_WFRB_ENQUEUEOP_H_
#define TERVEL_WFRB_ENQUEUEOP_H_

#include "tervel/wf-ring-buffer/helper.h"
#include "tervel/util/info.h"
#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/rc/descriptor_util.h"

#include <algorithm>
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

  explicit EnqueueOp<T>() {}

  ~EnqueueOp<T>() {}

  bool execute();

  /**
   * This function overrides the virtual function in the OpRecord class
   * It is called by the progress assurance scheme.
   */
  void help_complete();

  /**
   *
   * @return True if watched
   */
  bool on_is_watched();

 //private:
};  // EnqueueOp class

/**
 *
 */
template<class T>
EnqueueOp::execute() {
  RingBuffer::wf_enqueue(this);
}

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_ENQUEUEOP_H_
