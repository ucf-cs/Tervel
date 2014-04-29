#include "tervel/util/hp/hazard_pointer.h"

namespace tervel {
namespace util {
namespace memory {
namespace hp {

inline bool HazardPointer::watch(SlotID slot, HPElement *descr,
      std::atomic<void *> *address, void *expected,
      HazardPointer *hazard_pointer) {
  hazard_pointer->watch(slot, descr);

  if (address->load() != expected) {
    hazard_pointer->clear_watch(slot);
    return false;
  } else {
    bool success = descr->on_watch(address, expected);
    if (success) {
      return true;
    } else {
      hazard_pointer->clear_watch(slot);
      return false;
    }
  }
}


bool HazardPointer::watch(SlotID slot, void *value,
      std::atomic<void *> *address, void *expected,
      HazardPointer *hazard_pointer) {
  hazard_pointer->watch(slot, value);

  if (address->load() != expected) {
    hazard_pointer->clear_watch(slot);
    return false;
  } else {
    return true;
  }
}

void HazardPointer::unwatch(SlotID slot, HPElement *descr
      , HazardPointer *hazard_pointer) {
  hazard_pointer->clear_watch(slot);
  descr->on_unwatch();
}

void HazardPointer::unwatch(SlotID slot) {
  hazard_pointer->clear_watch(slot);
}


bool HazardPointer::is_watched(HPElement *descr
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {
  if (hazard_pointer->contains(descr)) {
    return descr->on_is_watched();
  } else {
    return true;
  }
}

bool HazardPointer::is_watched(void *value
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {
  return hazard_pointer->contains(value);
}

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel
