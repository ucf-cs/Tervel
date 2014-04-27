#ifndef TERVEL_MEMORY_HP_HAZARD_POINTER_H_
#define TERVEL_MEMORY_HP_HAZARD_POINTER_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>
#include <stddef.h>


#include "tervel/memory/system.h"
#include "tervel/util.h"

namespace tervel {
namespace memory {
namespace hp {

class HazardPointer {
  // REVIEW(carlos): class comment should go above class name/opening bracket
  /**
   * This class is used to maintain the list of hazard pointed objects.
   * Any value can be written into a slot, however we provide special
   * implementation for HPElements, in that we call their on_* functions.
   * This allows for more expressive operations to be performed.
   * 
   * If an individual thread requires more than one element to be hazard pointer
   * portected at a single instance, then SlotIDs should be added.
   */
 public:
  enum class SlotID {SHORTUSE, END};

  // REVIEW(carlos): parameter naming should be all_lower_case.
  explicit HazardPointer(int nThreads)
      // REVIEW(carlos): compiler errors: should be SlotId::END
      // REVIEW(carlos): Use spaces around mathematical operators
      // REVIEW(carlos): compiler errors: multiplying an int and an 'enum class'
      //   is not allowed because enum classes don't autoconvert to int. If you
      //   need it, you either have to static_cast<> or use regular enum's
      //   see: http://stackoverflow.com/questions/8357240
      //   Further, there really needs to be a constructor comment saying
      //   why you would want to do that in the first place, because it seems
      //   like a hack to me.
      : num_slots {nThreads*SlotID:END}
      // REVIEW(carlos): compiler error: 'value' is not defined anywhere as a
      //   type. Did you meant to make HazardPointer a template class?
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

  // REVIEW(carlos): shouldn't be a blank line between the documentation comment
  //   and the function.
  // REVIEW(carlos): For the rest of the functions as well: line continuations
  //   should just be indented 2 levels (4 spaces) on the next line. Don't
  //   bother trying to line things up.
  // REVIEW(carlos): The leading comma is generally only used in multiline
  //   constructor initializers to counter the visual weight of the colon. I
  //   don't really advise its use it in this case.
  // REVIEW(carlos): For default params, I typically like not to use spaces
  //   around the equal sign. Doesn't really matter so long as it's consistent
  //   within the file, though.
  // REVIEW(carlos): compiler error: unknown type HPElement. Add the forward
  //   declaration at the top of the file. As a side note, forward declarations
  //   are preferred to header inclusions whenever possible (when all you want
  //   is a pointer) to reduce compile time.
  // REVIEW(carlos): compiler error: tl_thread_info not found. You'll need
  //   another forward declaration at the top:
  //     namespace tervel {
  //     namespace memory {
  //
  //     extern thread_local tl_thread_info;
  //     ...
  //
  //     }  // namespace memory
  //     }  // namespace tervel
  // REVIEW(carlos): for all following functions: I was a bit confused for a
  //   minute on naming because all the functions taking a SlotID called that
  //   parameter `slot', and I didn't understand the difference between a slot
  //   and a slot id. Consider renaming them from slot to slot_id
  // REVIEW(carlos): tl_thread_info is not a pointer, accessing members is done
  //   using dots, not arrows.
  static bool watch(SlotID slot, HPElement *elem, std::atomic<void *> *address
             , void *expected
             , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer);

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
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer);

  /**
   * This method is used to remove the hazard pointer watch.
   * If a descr is passed then it will internally call descr->on_unwatch.
   *
   *
   * @param slot the slot to remove the watch
   */
  static void unwatch(SlotID slot
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer);

  /**
   * This method is used to remove the hazard pointer watch.
   * If a descr is passed then it will internally call descr->on_unwatch.
   *
   *
   * @param slot the slot to remove the watch
   * @param descr to call on_unwatch on.
   */
  static void unwatch(SlotID slot, HPElement *descr
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer);

  /**
   * This method is used to determine if a hazard pointer watch exists on a
   * passed value.
   * If a descr is passed then it will internally call descr->on_is_watched.
   *
   * @param descr to call on_is_watched on.
   */
  static bool is_watched(HPElement *descr
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer);

  /**
   * This method is used to determine if a hazard pointer watch exists on a
   * passed value.
   * If a descr is passed then it will internally call descr->on_is_watched.
   *
   * @param value to check if watch
   */
  static bool is_watched(void *value
            , HazardPointer *hazard_pointer = tl_thread_info->hazard_pointer);


  // -------
  // Member Functions
  // -------
  // REVIEW(carlos): You do this for all below functions: don't put a semi colon
  // after the closing brace of the body of a function.

  // REVIEW(carlos): Comments should start on line after the double star.
  /** This function takes a SlotID and stores the specified value into that
   * the threads alloated slot for that id in the hazard pointer watch list
   *
   * @param slot The id of the slot to watch.
   * @param value The value to watch
   **/
  void watch(SlotID slot, void *value) {
    // REVIEW(carlos): enum classes don't support automatic casting to int (the
    //   return type of get_slot). Don't assign the result of get_slot to slot,
    //   and just use the returned value directly in the operator[] of watches_:
    //     watches_[get_slot(slot)].store(value);
    slot = get_slot(slot);
    watches_[slot].store(value);
  };

  // REVIEW(carlos): Comments should start on line after the double star.
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
   * @return true is the table contains the specified value
   */

  // REVIEW(carlos): unwanted blank line between doc comment and function.
  bool contains(void *value) {
    for (int i = 0; i < num_slots; i++) {
      if (watches_[i].load() == value) {
        return true;
      }
    }
    return false;
  };

 private:
  // REVIEW(carlos): text should start on line after the double star
  /** This function calculates a the position of a threads slot for the
   * specified SlotID
   *
   * @param slot The slot id to get the position of
   */
  // REVIEW(carlos): Since get_slot returns an index into an array, consider
  //   returning a size_t instead.
  int get_slot(SlotID id) {
    // REVIEW(carlos): compiler error: SlotIDs:end -> SlotID::END
    return id + (SlotIDs:end * tl_thread_info.thread_id);
  };

  // REVIEW(carlos): compiler error: you have to #include <memory> to use
  //   std::unique_ptr
  std::unique_ptr<std::atomic<value *>[]> watches_;
  // REVIEW(carlos): naming: member vars should have a trailing underscore
  const size_t num_slots;

  DISALLOW_COPY_AND_ASSIGN(HazardPointer);
};


}  // namespace hp
}  // namespace memory
}  // namespace tervel

#endif  // TERVEL_MEMORY_HP_HAZARD_POINTER_H_
