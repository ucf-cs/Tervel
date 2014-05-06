#ifndef TERVEL_WFRB_HELPER_H_
#define TERVEL_WFRB_HELPER_H_

#include "tervel/util/info.h"
#include "tervel/util/util.h"

namespace tervel {
namespace wf_ring_buffer {
/**
 * TODO(ATB) insert class description
 */
template<class T>
class Helper : public util::Descriptor {
 public:
  explicit Helper<T>(T val, unsigned long seq)
      : val_(val)
      , seq_(seq) {}

  ~Helper<T>() {
    // TODO call OpRec safeFree(true) if not null
  }

  using util::Descriptor::on_watch;
  bool on_watch(std::atomic<void*> *address, void *value) {
    typedef util::memory::HazardPointer::SlotID t_SlotID;
    bool success = util::memory::hp::HazardPointer::watch(t_SlotID::SHORTUSE,
                                                          buffer_op,
                                                          address,
                                                          value);
    if (success) {
      Helper<T> *curr_node = buffer_->
    }
  }

  using util::Descriptor::on_is_watched;
  bool on_is_watched() {

  }

 //private:
};  // Helper class

}  // namespace wf_ring_buffer
}  // namespace tervel
#endif  // TERVEL_WFRB_HELPER_H_