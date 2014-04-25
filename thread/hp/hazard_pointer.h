#ifndef UCF_THREAD_HP_HAZARD_POINTER_H_
#define UCF_THREAD_HP_HAZARD_POINTER_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include "thread/descriptor.h"
#include "thread/system.h"

namespace ucf {
namespace thread {
namespace hp {

class HazardPointer {
 public:
  // REVIEW(carlos) enums should be named like classes; their memebers like
  //   constants.
  enum class SlotID{
    kop_rec
    ,kid_temp  // a better name is needed, but it basically means that it is
    // used temportalliy to gain a stronger watch
    ,kend
  };

  explicit HazardPointer(int nThreads)
      : num_slots {nThreads*SlotID:kend}
      , watches_(new std::atomic<value *>[num_slots]) {}


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

  static bool watch(SlotID slot, HPElement *descr, std::atomic<void *> *address
             , void *expected
             , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {

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
  static bool watch(SlotID slot, void *value, std::atomic<void *> *address
            , void *expected
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {

  /**
   * This method is used to remove the hazard pointer watch.
   * If a descr is passed then it will internally call descr->on_unwatch.
   *
   *
   * @param slot the slot to remove the watch
   * @param (optional) descr to call on_unwatch on.
   */
  static void unwatch(SlotID slot
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {
  static void unwatch(SlotID slot, HPElement *descr
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {

  /**
   * This method is used to determine if a hazard pointer watch exists on a passed
   * value.
   * If a descr is passed then it will internally call descr->on_is_watched.
   *
   * @param (optional) descr to call on_is_watched on.
   */
  static bool is_watched(HPElement *descr
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {
  static bool is_watched(void *value
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer) {


  // -------
  // Member Functions
  // -------

  /** This function takes a SlotID and stores the specified value into that
   * the threads alloated slot for that id in the hazard pointer watch list 
   * 
   * @param slot The id of the slot to watch.
   * @param value The value to watch
   **/
  void watch(SlotID slot, void *value) {
    slot = get_slot(slot);
    watches_[slot].store(value);
  };

  /** This function takes a SlotID and stores null into that
   * the threads alloated slot for that id in the hazard pointer watch list 
   *
   * @param slot The id of the slot to watch.
   */
  void clear_watch(SlotID slot) {
    slot = get_slot(slot);
    watches_[slot].store(nullptr);
  };


  /** This function returns true of the specified value is being watched.
   *
   * @param value The value to check.
   */

  bool contains(void *value) {
    for (int i = 0; i < num_slots; i++) {
      if (watches_[i].load() == value) {
        return true;
      }
    }
    return false;
  };

 private:
  /** This function calculates a the position of a threads slot for the
   * specified SlotID
   * 
   * @param slot The slot id to get the position of
   */
  int get_slot(SlotID id) {
    return id + (SlotIDs:end * tl_thread_info.thread_id);
  };

  std::unique_ptr<std::atomic<value *>[]> watches_;
  const size_t num_slots;
};


}  // namespace hp
}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_HP_HAZARD_POINTER_H_
