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

template<class T>
class EnqueueOp : public util::OpRecord {
 public:

  explicit EnqueueOp<T>() {}

  ~EnqueueOp<T>() {}

  /**
   * Todo: Enqueues a value
   */
  bool execute();

  /**
   * This function overrides the virtual function in the OpRecord class
   * It is called by the progress aurrance scheme.
   */
  void help_complete();

  /**
   *
   * @return True if watecd
   */
  bool on_is_watched();

 //private:
};  // EnqueueOp class

/**
 *
 */
template<class T>
EnqueueOp::execute() {
    tervel::util::ProgressAssurance::check_for_announcement();
  // TODO enqueue
}

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_ENQUEUEOP_H_
