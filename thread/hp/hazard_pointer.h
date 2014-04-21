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
      static enum slot_id {
        op_rec,
        id_temp, // a better name is needed, but it basically means that it is
        // used temportalliy to gain a stronger watch
        end
      };

      explicit HazardPointer(int nThreads)
        : num_slots {nThreads*slot_id:end},
          watches_(new std::atomic<value *>[num_slots]) {}

        void watch(int slot, void *value) {
          watches_[slot].store(value);
        };

        void clear_watch(int slot) {
          watches_[slot].store(nullptr);
        };

        int get_slot(slot_id id) {
          return id + (slot_ids:end * tl_thread_info.thread_id);
        };

        bool contains(void *v) {
          for (int i = 0; i < num_slots; i++) {
            if (watches_[i].load() == v) {
              return true;
            }
          }
          return false;
        };

    private:
        std::unique_ptr<std::atomic<value *>[]> watches_;
        const size_t num_slots;
  };


}  // namespace hp
}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_HP_HAZARD_POINTER_H_
