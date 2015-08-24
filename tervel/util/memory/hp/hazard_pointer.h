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
#ifndef TERVEL_MEMORY_HP_HAZARD_POINTER_H_
#define TERVEL_MEMORY_HP_HAZARD_POINTER_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stddef.h>
#include <memory>
#include <cstdint>

#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/memory/hp/list_manager.h>

namespace tervel {
namespace util {
namespace memory {
namespace hp {

class Element;

/**
 * This class is used to maintain the list of hazard pointed objects.
 * Any value can be written into a slot, however we provide special
 * implementation for Elements, in that we call their on_* functions.
 * This allows for more expressive operations to be performed.
 *
 * If an individual thread requires more than one element to be hazard pointer
 * protected at a single instance, then SlotIDs should be added.
 */
class HazardPointer {
 public:
  enum class SlotID : size_t {SHORTUSE = 0, SHORTUSE2, PROG_ASSUR, END};

  explicit HazardPointer(int num_threads);
  ~HazardPointer();

  // -------
  // Static Functions
  // -------

  /**
   * This method is used to achieve a hazard pointer watch on the the based descr.
   * Internally it will call the descriptors on_watch function.
   *
   * If after writing descr the object is still at the address
   * (indicated by *a == value), it will call on_watch.
   * If that returns true then it will return true.
   * Otherwise it removes the hazard pointer watch and returns false
   *
   * @param slot The position to place the descr value in the watch table.
   * @param descr The descr that is to be watched.
   * @param address The address to check
   * @param expected The value which is to be expected at the address
   */
  static bool watch(SlotID slot_id, Element *elem, std::atomic<void *> *address,
        void *expected, HazardPointer * const hazard_pointer =
        tervel::tl_thread_info->get_hazard_pointer());

  /**
   * This method is used to achieve a hazard pointer watch on a memory address.
   *
   * If after writing the value, it is still at the address
   * (indicated by *a == value), will return true.
   * Otherwise it removes the hazard pointer watch and returns false
   *
   * @param slot The position to place the value in the watch table.
   * @param value The value that is to be watched.
   * @param address The address to check
   * @param expected The value which is to be expected at the address
   */
  static bool watch(SlotID slot_id, void *value, std::atomic<void *> *address
      , void *expected, HazardPointer * const hazard_pointer =
      tervel::tl_thread_info->get_hazard_pointer());

  /**
   * This method is used to remove the hazard pointer watch.
   * If a descr is passed then it will internally call descr->on_unwatch.
   *
   * @param slot the slot to remove the watch
   */
  static void unwatch(SlotID slot_id, HazardPointer * const hazard_pointer =
        tervel::tl_thread_info->get_hazard_pointer());

  /**
   * This method is used to determine if a thread has a hazard pointer watch.
   *
   * @param slot the slot to remove the watch
   */
  static bool hasWatch(SlotID slot_id, HazardPointer * const hazard_pointer =
        tervel::tl_thread_info->get_hazard_pointer());

  /**
   * This method is used to remove the hazard pointer watch.
   * If a descr is passed then it will internally call descr->on_unwatch.
   *
   * @param slot the slot to remove the watch
   * @param descr to call on_unwatch on.
   */
  static void unwatch(SlotID slot_id, Element *descr,
          HazardPointer * const hazard_pointer =
          tervel::tl_thread_info->get_hazard_pointer());

  /**
   * This method is used to determine if a hazard pointer watch exists on a
   * passed value.
   * If a descr is passed then it will internally call descr->on_is_watched.
   *
   * @param descr to call on_is_watched on.
   */
  static bool is_watched(Element *descr, HazardPointer * const hazard_pointer =
        tervel::tl_thread_info->get_hazard_pointer());

  /**
   * This method is used to determine if a hazard pointer watch exists on a
   * passed value.
   * If a descr is passed then it will internally call descr->on_is_watched.
   *
   * @param value to check if watch
   */
  static bool is_watched(void *value, HazardPointer * const hazard_pointer =
        tervel::tl_thread_info->get_hazard_pointer());


  // -------
  // Member Functions
  // -------
  /**
   * This function takes a SlotID and stores the specified value into that
   * the threads alloted slot for that id in the hazard pointer watch list
   *
   * @param slot The id of the slot to watch.
   * @param value The value to watch
   **/
  void watch(SlotID slot, void *value) {
    int temp = get_slot(slot);
    assert(watches_[temp].load() == nullptr);
    watches_[temp].store(value);
    assert(watches_[temp].load() == value);
  }

  void * value(SlotID slot) {
    return watches_[get_slot(slot)].load();
  }

  /**
   * This function takes a SlotID and stores null into that
   * the threads alloted slot for that id in the hazard pointer watch list
   *
   * @param slot The id of the slot to watch.
   */
  void clear_watch(SlotID slot) {
    watches_[get_slot(slot)].store(nullptr);
  }


  /**
   * This function returns true of the specified value is being watched.
   *
   * @param value The value to check.
   * @return true is the table contains the specified value
   */
  bool contains(void *value) {
    for (size_t i = 0; i < num_slots_; i++) {
      if (watches_[i].load() == value) {
        return true;
      }
    }
    return false;
  }

 private:
  /**
   * This function calculates a the position of a threads slot for the
   * specified SlotID
   *
   * @param slot The slot id to get the position of
   */
  size_t get_slot(SlotID id) {
    size_t s = static_cast<size_t>(id) + (static_cast<size_t>(SlotID::END) *
          tervel::tl_thread_info->get_thread_id());
    assert(s < num_slots_);
    return s;
  }

  std::unique_ptr<std::atomic<void *>[]> watches_;
  const size_t num_slots_;

 public:
  // Shared HP Element list manager
  util::memory::hp::ListManager hp_list_manager_;


  DISALLOW_COPY_AND_ASSIGN(HazardPointer);
};  // HazardPointer


}  // namespace hp
}  // namespace memory
}  // namepsace UTIL
}  // namespace tervel

#endif  // TERVEL_MEMORY_HP_HAZARD_POINTER_H_
