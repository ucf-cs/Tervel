/*
The MIT License (MIT)

Copyright (c) 2015 University of Central Florida's Computer Software Engineering
Scalable & Secure Systems (CSE - S3) Lab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <tervel/util/memory/hp/hazard_pointer.h>
#include <tervel/util/memory/hp/hp_element.h>

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
  , hp_list_manager_(num_threads) {
    for (size_t i = 0; i < num_slots_; i++) {
      watches_[i].store(nullptr);
    }
  }

HazardPointer::~HazardPointer() {
  for (size_t i = 0; i < num_slots_; i++) {
    assert(watches_[i].load() == nullptr && "Some memory is still being watched and hazard pointer construct has been destroyed");
  }
  // delete watches_; // std::unique_ptr causes this array to be destroyed
}

bool HazardPointer::watch(SlotID slot, Element *descr,
      std::atomic<void *> *address, void *expected,
      HazardPointer * const hazard_pointer) {
  #ifdef TERVEL_MEM_HP_NO_WATCH
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
  #ifdef TERVEL_MEM_HP_NO_WATCH
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
  #ifdef TERVEL_MEM_HP_NO_WATCH
    return;
  #endif
  hazard_pointer->clear_watch(slot);
  descr->on_unwatch();
}

bool HazardPointer::hasWatch(SlotID slot,
    HazardPointer * const hazard_pointer) {
  #ifdef TERVEL_MEM_HP_NO_WATCH
    return false;
  #endif
  return nullptr != hazard_pointer->value(slot);

}
void HazardPointer::unwatch(SlotID slot, HazardPointer * const hazard_pointer) {
  #ifdef TERVEL_MEM_HP_NO_WATCH
    return;
  #endif
  hazard_pointer->clear_watch(slot);
}


bool HazardPointer::is_watched(Element *descr,
  HazardPointer * const hazard_pointer) {
  #ifdef TERVEL_MEM_HP_NO_WATCH
    return false;
  #endif
  if (hazard_pointer->contains(descr)) {
    return true;
  }
  return descr->on_is_watched();
}

bool HazardPointer::is_watched(void *value,
  HazardPointer * const hazard_pointer) {
  #ifdef TERVEL_MEM_HP_NO_WATCH
    return false;
  #endif
  return hazard_pointer->contains(value);
}

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel
