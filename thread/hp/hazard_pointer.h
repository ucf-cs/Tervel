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

// REVIEW(carlos): indentation level is off:
//   + no extra indentation inside a namespace
//   + access modifiers in class indented 1 space
//   + everything else in a class indented 2 spaces
  class HazardPointer {
    public:
      // REVIEW(carlos) static modifier makes no sense for enums.
      // REVIEW(carlos) consider using C++11 enum classes
      // REVIEW(carlos) enums should be named like classes; their memebers like
      //   constants.
      static enum slot_id {
        op_rec,
        id_temp,  // a better name is needed, but it basically means that it is
        // used temportalliy to gain a stronger watch
        end
      };

      // REVIEW(carlos) 2 indentation levels (4 spaces) when initializer list
      //   gets pushed to next line. I also like to start each line with the
      //   comma seperator (for aesthetics)
      //   ... HazardPointer(...)
      //       : num_slots {...}
      //       , watches(...) {}
      explicit HazardPointer(int nThreads)
        : num_slots {nThreads*slot_id:end},
          watches_(new std::atomic<value *>[num_slots]) {}

      // REVIEW(carlos) not strictly chosen yet, but should use java-style
      // double stared comments for function comments.

      /* This function takes a slot_id and stores the specified value into that
       * the threads alloated slot for that id in the hazard pointer watch list 
       * REVIEW(carlos) should be a blank line between function description and
       *   param directives.
       * REVIEW(carlos) Should be @param, not @params.
       * REVIEW(carlos) The first word of the param directive is the param name,
       *   everything else is a description. ex:
       *     @param slot The id of the slot to watch.
       *     @param value I have no idea what this is.
       *   I'd omit the description if it's obvious.
       * REVIEW(carlos) 1 space between the @param and the star, not 2
       *  @params a slot id, and value
       */
      void watch(slot_id slot, void *value) {
        slot = get_slot(slot);
        watches_[slot].store(value);
      };

      /* This function takes a slot_id and stores null into that
       * the threads alloated slot for that id in the hazard pointer watch list 
       * REVIEW(carlos) see comments on watch().
       *  @params a slot_id
       */
      void clear_watch(slot_id slot) {
        slot = get_slot(slot);
        watches_[slot].store(nullptr);
      };


      /* This function returns true of the specified value is being watched.
       * REVIEW(carlos) see comments on watch().
       *  @params a value
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
      /* This function calculates a the position of a threads slot for the
       * specified slot_id
       * REVIEW(carlos) see comments on watch().
       *  @params a slot_id
       */
      int get_slot(slot_id id) {
        return id + (slot_ids:end * tl_thread_info.thread_id);
      };

      // REVIEW(carlos) shouldn't have extra indentation for class members
        std::unique_ptr<std::atomic<value *>[]> watches_;
        const size_t num_slots;
  };


}  // namespace hp
}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_HP_HAZARD_POINTER_H_
