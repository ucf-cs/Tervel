#ifndef TERVEL_WFRB_RINGBUFFER_H_
#define TERVEL_WFRB_RINGBUFFER_H_

#include "tervel/wf-ring-buffer/wf_ring_buffer_helper.h"
#include "tervel/util/info.h"
#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/rc/descriptor_util.h"

#include <algorithm>
#include <cstdint>

namespace tervel {
namespace wf_ring_buffer {
/**
 * TODO class desc
 */
template<class T>
class RingBuffer : public util::memory::hp::Element {
 public:
  static constexpr T MCAS_FAIL_CONST = reinterpret_cast<T>(0x1L);

  explicit RingBuffer<T>(int capacity, int num_threads)
      : capacity_(capacity)
      , num_threads_(num_threads) {}

  ~RingBuffer<T>() {
    for (int i = 0; i < num_threads_; i++) {
      Helper<T>* helper = helper_table_[i].helper_.load();
      // The No check flag is true because each was check prior
      // to the call of this descructor.
      if (helper == MCAS_FAIL_CONST) {
        break;
      }
      util::memory::rc::free_descriptor(helper, true);
    }
  }

  /**
   * TODO: Initializes buffer...
   */
  bool init();

  /**
   * TODO: Enqueues buffer element...
   */
  bool enqueues();

  /**
   * TODO: Dequeues buffer element...
   */
  bool dequeue();

private:
  int capacity_;
  int num_threads_;
};  // RingBuffer class

template<class T>
RingBuffer::init() {
  buffer_ = new T[capacity_];
  for (int i = 0; i < capacity_; i++) {
    buffer_[i] = new EmptyHelper(i);
  }
  rec_table_ = new OpRec[num_threads_];
  for (int i = 0; i < num_threads_; i++) {
    rec_table_[i] = new OpRec();
  }
}

template<class T>
bool RingBuffer::enqueue() {

}

template<class T>
bool RingBuffer::dequeue() {

}

}  // namespace mcas
}  // namespace tervel

#endif  // TERVEL_WFRB_RINGBUFFER_H_
