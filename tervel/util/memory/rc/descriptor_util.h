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
#ifndef TERVEL_MEMORY_RC_UTIL_DESCRIPTOR_H_
#define TERVEL_MEMORY_RC_UTIL_DESCRIPTOR_H_

#include <tervel/util/info.h>
#include <tervel/util/descriptor.h>

#include <tervel/util/memory/rc/descriptor_pool.h>
#include <tervel/util/memory/rc/pool_element.h>

#include <tervel/util/memory/hp/hazard_pointer.h>

#include <tervel/util/recursive_action.h>
#include <tervel/util/progress_assurance.h>

#include <tervel/util/tervel_metrics.h>
namespace tervel {
namespace util {

class Descriptor;

namespace memory {
namespace rc {


/**
 * @brief Constructs and returns a descriptor. Arguments are forwarded to the
 * constructor of the given descriptor type. User should call free_descriptor
 * on the returned pointer when they are done with it to avoid memory leaks.
 */
template<typename DescrType, typename... Args>
inline DescrType * get_descriptor(Args&&... args) {
  auto rc_descr_pool = tervel::tl_thread_info->get_rc_descriptor_pool();
  return rc_descr_pool->get_descriptor<DescrType>(std::forward<Args>(args)...);
}

/**
 * @brief Once a user is done with a descriptor, they should free it with this
 * method.
 *
 * @param descr The descriptor to free.
 * @param dont_check Don't check if the descriptor is being watched before
 *   freeing it. Use this flag if you know that no other thread has had access
 *   to this descriptor.
 * @param pool the pool to use when freeing the descriptor.
 */
inline void free_descriptor(tervel::util::Descriptor *descr,
      bool dont_check = false) {
  tervel::tl_thread_info->get_rc_descriptor_pool()->free_descriptor(descr,
        dont_check);
}

inline void safefree_descriptor(tervel::util::Descriptor *descr) {
  free_descriptor(descr, false);
}

inline void unsafefree_descriptor(tervel::util::Descriptor *descr) {
  free_descriptor(descr, true);
}

/**
* @brief This method is used to determine if the passed descriptor is under rc
* protection.
* Internally calls on_is_watched
*
* @param descr the descriptor to be checked for rc protection.
*/
inline bool is_watched(tervel::util::Descriptor *descr) {
  #ifdef TERVEL_MEM_RC_NO_WATCH
  return false;
  #endif  // TERVEL_MEM_RC_NO_WATCH
  PoolElement * elem = get_elem_from_descriptor(descr);
  int64_t ref_count = elem->header().ref_count.load();
  assert(ref_count >=0 && " Ref count of an object is negative, which implies some thread called unwatch multiple times on the same object");
  if (ref_count == 0) {
    return descr->on_is_watched();
  } else {
    return true;
  }
}

/**
* @brief This method is used to increment the reference count of the passed descriptor
* object. If after incrementing the reference count the object is still at the
* address (indicated by *a == value), it will call on_watch.
* If that returns true then it will return true.
* Otherwise it decrements the reference count and returns false
* Internally calls on_watch
*
* @param descr the descriptor which needs rc protection
* @param address address it was dereferenced from
* @param val the read value of the address
* @return true if successfully acquired a watch
*/
inline bool watch(tervel::util::Descriptor *descr, std::atomic<void *> *address,
        void *value) {
  #ifdef TERVEL_MEM_RC_NO_WATCH
    return true;
  #endif  // TERVEL_MEM_RC_NO_WATCH

  PoolElement *elem = get_elem_from_descriptor(descr);
  elem->header().ref_count.fetch_add(1);

  if (address->load() != value) {
    int64_t temp = elem->header().ref_count.fetch_add(-1);
    assert(temp > 0 && " Ref count of an object is negative, which implies some thread called unwatch multiple times on the same object");
    #if tervel_track_rc_watch_fail  == tervel_track_enable
      TERVEL_METRIC(rc_watch_fail)
    #endif
    return false;
  } else {
    bool res = descr->on_watch(address, value);
    if (res) {
      assert(is_watched(descr) && "On watch returned true, but the object is not watched. Error could exist on either [on_]watch or [on_]is_watched functions");
     return true;
    } else {
      int64_t temp = elem->header().ref_count.fetch_add(-1);
      assert(temp > 0 && " Ref count of an object is negative, which implies some thread called unwatch multiple times on the same object");
      return false;
    }
  }
}

/**
* @brief This method is used to decrement the reference count of the passed descriptor
* object.
* Then it will call on_unwatch and decrement any related objects necessary
* Internally calls on_unwatch
*
* @param descr the descriptor which no longer needs rc protection.
*/
inline void unwatch(tervel::util::Descriptor *descr) {
  #ifdef TERVEL_MEM_RC_NO_WATCH
    return;
  #endif  // TERVEL_MEM_RC_NO_WATCH

  PoolElement *elem = get_elem_from_descriptor(descr);
  int64_t temp = elem->header().ref_count.fetch_add(-1);
  assert(temp > 0 && " Ref count of an object is negative, which implies some thread called unwatch multiple times on the same object");
  descr->on_unwatch();
}

/**
 * @brief This returns the passed reference with its least signifcant bit set
 * to 1.
 *
 * @param descr the reference to bitmark
 * @return the bitmarked reference
 */
inline void * mark_first(tervel::util::Descriptor *descr) {
  return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(descr) | 0x1L);
}

/**
 * @brief This function atomically bit marks the value at address
 * @details This function atomically bit marks the value at address
 *
 * @param address the address to bitmark
 */
inline void atomic_mark_first(std::atomic<void*> *address) {
  std::atomic<uintptr_t> *temp = reinterpret_cast<std::atomic<uintptr_t> *>(address);
  temp->fetch_or(0x1);
}

/**
 * @brief This returns an unbitmarked reference
 *
 * @param the reference to remove the bitmark from
 * @return the unbitmarked reference
 */
inline tervel::util::Descriptor * unmark_first(void *descr) {
  return reinterpret_cast<tervel::util::Descriptor *>(
      reinterpret_cast<uintptr_t>(descr) & ~0x1L);
}

/**
 * @brief This returns whether or not the least significant bit holds a bitmark
 *
 * @param the reference to check
 * @return whether or not it holds a bitmark
 */
inline bool is_descriptor_first(void *descr) {
  return (0x1L == (reinterpret_cast<uintptr_t>(descr) & 0x1L));
}

/**
* @brief This method is used to remove a descriptor object that is conflict with
* another threads operation.
* It first checks the recursive depth before proceeding.
* Next it protects against the object being reused else where by acquiring
* either HP or RC watch on the object.
* Then once it is safe it will call the objects complete function
* This function must guarantee that after its return the object has been removed
* It returns the value.
*
* @param expected a marked reference to the object
* @param address the location expected was read from
* dereferenced from.
* @return the current value of the address
*/
inline void * remove_descriptor(void *expected, std::atomic<void *> *address) {
  assert(util::memory::hp::HazardPointer::hasWatch(util::memory::hp::HazardPointer::SlotID::SHORTUSE) == false && "Thread did not release all HP watches and may-reuse a SHORTUSE watch");

  tervel::util::RecursiveAction recurse;
  void *newValue;
  if (tervel::util::RecursiveAction::recursive_return()) {
    newValue = nullptr;  // result not used
  } else {
    tervel::util::Descriptor *descr = unmark_first(expected);
    if (watch(descr, address, expected)) {
      assert(is_watched(descr) && "On watch returned true, but the object is not watched. Error could exist on either [on_]watch or [on_]is_watched functions");
      newValue = descr->complete(expected, address);

      #if tervel_track_max_rc_remove_descr  == tervel_track_enable
        TERVEL_METRIC(rc_remove_descr)
      #endif
      unwatch(descr);
    } else {
      newValue = address->load();
    }
  }
  return newValue;
}

void *wf_descriptor_read_first(std::atomic<void *> *address);
/**
 * @brief This function determines the logical value of an address which may have
 * either a RC descriptor or a normal value.
 *
 *
 * @param address to read
 * @return the current logical value
 */
inline void *descriptor_read_first(std::atomic<void *> *address) {
  void *current_value = address->load();

  util::ProgressAssurance::Limit progAssur;


  while (is_descriptor_first(current_value)) {

    if (progAssur.isDelayed()) {
      return wf_descriptor_read_first(address);

    } else {
      tervel::util::Descriptor *descr = unmark_first(current_value);
      if (watch(descr, address, current_value)) {
        current_value = descr->get_logical_value();
        unwatch(descr);
      } else {
        current_value = address->load();
      }
    }
  }
  return current_value;
}

}  // namespace rc
}  // namespace memory
}  // namespace util
}  // namespace tervel


#endif  // TERVEL_MEMORY_RC_UTIL_DESCRIPTOR_H_
