#include "tervel/util/memory/hp/hazard_pointer.h"
#include "tervel/util/memory/hp/hp_element.h"

namespace tervel {
namespace util {
namespace memory {
namespace hp {

HazardPointer::HazardPointer(int num_threads)
  // The total number of slots needed is equal to the number of threads
  // multiples by the number of slots used.
  // Do to the potential of reordering, num_slots_ can not be used to
  // initialize watches.
  : watches_(new std::atomic<void *>[num_threads *
        static_cast<size_t>(SlotID::END)])
  , num_slots_ {num_threads * static_cast<size_t>(SlotID::END)}
  , hp_list_manager_(num_threads) {}

HazardPointer::~HazardPointer() {
  for (int i = 0; i < num_slots_; i++) {
    assert(watches_[i].load() != nullptr && "Some memory is still being watched and hazard pointer construct has been destroyed");
  }
  // delete watches_; // std::unique_ptr causes this array to be destroyed
}

bool HazardPointer::watch(SlotID slot, Element *descr,
      std::atomic<void *> *address, void *expected,
      HazardPointer * const hazard_pointer) {
  #ifdef NOMEMORY
    #error this could be it?
    return true;
  #endif
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
      HazardPointer * const hazard_pointer) {
  #ifdef NOMEMORY
    return true;
  #endif

  hazard_pointer->watch(slot, value);

  if (address->load() != expected) {
    hazard_pointer->clear_watch(slot);
    return false;
  } else {
    return true;
  }
}

void HazardPointer::unwatch(SlotID slot, Element *descr,
    HazardPointer * const hazard_pointer) {
  #ifdef NOMEMORY
    return;
  #endif
  hazard_pointer->clear_watch(slot);
  descr->on_unwatch();
}

void HazardPointer::unwatch(SlotID slot, HazardPointer * const hazard_pointer) {
  #ifdef NOMEMORY
    return;
  #endif
  hazard_pointer->clear_watch(slot);
}


bool HazardPointer::is_watched(Element *descr,
  HazardPointer * const hazard_pointer) {
  #ifdef NOMEMORY
    return false;
  #endif
  if (hazard_pointer->contains(descr)) {
    return true;
  }
  return descr->on_is_watched();
}

bool HazardPointer::is_watched(void *value,
  HazardPointer * const hazard_pointer) {
  #ifdef NOMEMORY
    return false;
  #endif
  return hazard_pointer->contains(value);
}

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel
