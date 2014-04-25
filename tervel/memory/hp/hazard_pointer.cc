#include "thread/hp/hazard_pointer.h"

namespace ucf {
namespace thread {
namespace hp {

bool watch(SlotID slot, HPElement *descr, std::atomic<void *> *address
           , void *expected
           , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer ) {
  hazard_pointer->watch(slot, descr);

  if (address->load() != expected) {
    hazard_pointer->clear_watch(slot);
    return false;
  } else {
    bool res = descr->on_watch(address, expected);
    if (res) {
      return true;
    } else {
      hazard_pointer->clear_watch(slot);
      return false;
    }
  }
}


bool watch(SlotID slot, void *value, std::atomic<void *> *address
          , void *expected
          , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {
  hazard_pointer->watch(slot, value);

  if (address->load() != expected) {
    hazard_pointer->clear_watch(slot);
    return false;
  } else {
    return true;
  }
}

void unwatch(SlotID slot, HPElement *descr
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {
  hazard_pointer->clear_watch(slot);
  descr->on_unwatch();
}

void unwatch(SlotID slot) {
  hazard_pointer->clear_watch(slot);
}


bool is_watched(HPElement *descr
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {
  if (hazard_pointer->contains(descr)) {
    return descr->on_is_watched();
  } else {
    return true;
  }
}

bool is_watched(void *value
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {
  return hazard_pointer->contains(value);
}

}  // namespace hp
}  // namespace thread
}  // namespace ucf
