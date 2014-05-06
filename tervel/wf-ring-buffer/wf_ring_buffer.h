#ifndef TERVEL_WFRB_WFRB_H_
#define TERVEL_WFRB_WFRB_H_

#include "tervel/wf-ring-buffer/wf_ring_buffer_helper.h"
#include "tervel/util/info.h"
#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/rc/descriptor_util.h"

#include <algorithm>
#include <cstdint>

namespace tervel {
namespace wf_ring_buffer {
/**
 * This is the MCAS class, it is used to perform a Multi-Word Compare-and-Swap
 * To execute an MCAS, simply call addCASTriple for each address you want to
 * update, then call execute();
 * This function is wait-free.
 */
template<class T>
class RingBuffer : public util::memory::hp::Element {
 public:
  static constexpr T MCAS_FAIL_CONST = reinterpret_cast<T>(0x1L);

  explicit RingBuffer<T>(int max_rows)
      : cas_rows_(new CasRow<T>[max_rows])
      , max_rows_ {max_rows} {}

  ~MCAS<T>() {
    for (int i = 0; i < row_count_; i++) {
      Helper<T>* helper = cas_rows_[i].helper_.load();
      // The No check flag is true because each was check prior
      // to the call of this descructor.
      if (helper == MCAS_FAIL_CONST) {
        break;
      }
      util::memory::rc::free_descriptor(helper, true);
    }
  }
};  // RingBuffer class

}  // namespace mcas
}  // namespace tervel

#endif  // TERVEL_MCAS_MCAS_H_
