#ifndef TERVEL_MEMORY_HP_POOL_ELEMENT_H_
#define TERVEL_MEMORY_HP_POOL_ELEMENT_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include "tervel/memory/descriptor.h"
#include "tervel/memory/system.h"

<<<<<<< HEAD:tervel/memory/hp/hp_element.h
namespace tervel {
namespace memory {

class Descriptor;

=======
namespace ucf {
namespace thread {
>>>>>>> code-review:thread/hp/hp_element.h
namespace hp {

/**
 * This class is used for the creation of Hazard Pointer Protected Objects
 * Objects which extend it have the ability to call safeFree which delays
 * the calling of the objects destructor until it is safe to do so.
 * 
 * To achieve more advance functionality, the user can also extend the Descriptor
 * class which will proive on_watch, on_unwatch, and on_is_watch functions.
 */
class HPElement {
 public:

  HPElement() {}
  ~HPElement() {}

  /** 
   * This function is used to free a hazard pointer protected object if it is
   * safe to do so OR add it to a list to be freed later.
   * It also calls 'try_to_free_HPElements' in an attempt to free previously
   * unfreeable objects.
   */
  void safeFree(HazardPointer *hazard_pointer = 
                                              tl_thread_info->hazard_pointer) {
    hazard_pointer->try_to_free_HPElements();
    if (HazardPointer::is_watched(this)) {
      hazard_pointer->add_to_unsafe(this);
    }
    else {
      this->~HPElement();
    }

    hazard_pointer->try_clear_unsafe_pool();

  }

  bool on_watch(std::atomic<void *> *address, void *expected) { return true };
  bool on_is_watch() {};
  void on_unwatch() {};


 private:
  HPElement *next_ {nullptr}
  /**
   * Helper method for getting the next pointer.
   */
  HPElement * next() { return next_; }

  /**
   * Helper method for setting the next pointer.
   */
  void next(HPElement *next) { next_ = next; }
  
};

}  // namespace hp
}  // namespace memory
}  // namespace tervel

#endif  // TERVEL_MEMORY_HP_POOL_ELEMENT_H_
