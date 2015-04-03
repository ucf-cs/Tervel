#include <tervel/util/memory/rc/descriptor_util.h>


namespace tervel {
namespace util {

class Descriptor;

namespace memory {
namespace rc {


void ReadFirstOp::help_complete() {
  void *aValue = value_.load();
  while (aValue == nullptr) {
    void *cvalue = address_->load();

    if (is_descriptor_first(cvalue)) {
      tervel::util::Descriptor *descr = unmark_first(cvalue);
      if (tervel::util::memory::rc::watch(descr, address_, cvalue)) {
        cvalue = descr->get_logical_value();
        unwatch(descr);
        value_.compare_exchange_strong(aValue, cvalue);
        return;
      } else {
        util::backoff();
        aValue = value_.load();
      }
    } else {
      value_.compare_exchange_strong(aValue, cvalue);
      return;
    }
  }
}

void *ReadFirstOp::load() {
  return value_.load();
}
}  // namespace rc
}  // namespace memory
}  // namespace util
}  // namespace tervel
