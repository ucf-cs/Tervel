#ifndef TERVEL_MEMORY_HP_HAZARD_POINTER_H_
#define TERVEL_MEMORY_HP_HAZARD_POINTER_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include "tervel/memory/descriptor.h"
#include "tervel/memory/system.h"

namespace tervel {
namespace memory {
namespace hp {

  class HazardPointer {
    public:
      static enum slot_id {
        op_rec,
        id_temp,  // a better name is needed, but it basically means that it is
        // used temportalliy to gain a stronger watch
        end
      };

      explicit HazardPointer(int nThreads)
        : num_slots {nThreads*slot_id:end},
          watches_(new std::atomic<value *>[num_slots]) {}

      /* This function takes a slot_id and stores the specified value into that
       * the threads alloated slot for that id in the hazard pointer watch list 
       *  @params a slot id, and value
       */
      void watch(slot_id slot, void *value) {
        slot = get_slot(slot);
        watches_[slot].store(value);
      };

      /* This function takes a slot_id and stores null into that
       * the threads alloated slot for that id in the hazard pointer watch list 
       *  @params a slot_id
       */
      void clear_watch(slot_id slot) {
        slot = get_slot(slot);
        watches_[slot].store(nullptr);
      };


      /* This function returns true of the specified value is being watched.
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
       *  @params a slot_id
       */
      int get_slot(slot_id id) {
        return id + (slot_ids:end * tl_thread_info.thread_id);
      };

        std::unique_ptr<std::atomic<value *>[]> watches_;
        const size_t num_slots;
  };


}  // namespace hp
}  // namespace memory
}  // namespace tervel

#endif  // TERVEL_MEMORY_HP_HAZARD_POINTER_H_
