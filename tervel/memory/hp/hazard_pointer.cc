// REVIEW(carlos): compiler error: include path isn't right:
//   "tervel/memory/hp/hazard_pointer.h"
//   NOTE: all include paths for tervel headers should start with tervel/
#include "thread/hp/hazard_pointer.h"

// REVIEW(carlos): should be namespace tervel
namespace ucf {
namespace memory {
namespace hp {

// REVIEW(carlos): All below comments apply to each function in this file
// REVIEW(carlos): See comments on line continuation in the h file.
// REVIEW(carlos): don't put the default argument in the cc file, just in the h
//   file. This avoids having 2 places where it's defined which may diverge in
//   the future.
// REVIEW(carlos): All the functions here should have the class name they belong
//   to prefixed. e.g., bool HazardPointer::watch(...) {
bool watch(SlotID slot, HPElement *descr, std::atomic<void *> *address
           , void *expected
           , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer ) {
  hazard_pointer->watch(slot, descr);

  if (address->load() != expected) {
    hazard_pointer->clear_watch(slot);
    return false;
  } else {
    // REVIEW(carlos): name res has no descriptive information. I would give it
    //   a more decriptive name or, if one does not exist, just put the function
    //   call in the condition of the if statement.
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
// REVIEW(carlos): should be namespace tervel
}  // namespace ucf
