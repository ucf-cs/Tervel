#ifndef TERVEL_UTIL_MEMORY_HP_POOL_ELEMENT_H_
#define TERVEL_UTIL_MEMORY_HP_POOL_ELEMENT_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include "tervel/util/descriptor.h"
#include "tervel/util/system.h"
#include "tervel/util/util.h"
#include "tervel/util/info.h"
#include "tervel/util/memory/hp/hazard_pointer.h"

namespace tervel {
namespace util {
namespace memory {
namespace hp {

class HazardPointer;
/**
 * This class is used for the creation of Hazard Pointer Protected Objects
 * Objects which extend it have the ability to call safeFree which delays
 * the calling of the objects destructor until it is safe to do so.
 * 
 * To achieve more advance functionality, the user can also extend Descriptor
 * class which will provides on_watch, on_unwatch, and on_is_watch functions.
 */
class Element {
 public:
  Element() {}

  /**
   * This function is used to free a hazard pointer protected object if it is
   * safe to do so OR add it to a list to be freed later.
   * It also calls 'try_to_free_Elements' in an attempt to free previously
   * unfreeable objects.
   */
  void safe_delete(bool no_check = false, HazardPointer *hazard_pointer =
          tervel::tl_thread_info->get_hazard_pointer()) {
    hazard_pointer->try_to_free_Elements();
    if (!no_check || HazardPointer::is_watched(this)) {
      hazard_pointer->add_to_unsafe(this);
    } else {
      delete this;
    }
    hazard_pointer->try_clear_unsafe_pool();
  }

  /**
   * This function is used to achieve a strong watch on an Element.
   * Classes wishing to express this should override this function.
   */
  virtual bool on_watch(std::atomic<void *> *address, void *expected) {
    return true;
  }

  /**
   * This function is used to check a strong watch on an Element.
   * Classes wishing to express this should override this function.
   */
  virtual bool on_is_watch() {}

  /**
   * This function is used to remove a strong watch on an Element.
   * Classes wishing to express this should override this function.
   */
  virtual void on_unwatch() {}


 private:
  // REVIEW(carlos): the getter and setter for next were public in
  //   DescriptorElement because they defined a public interface to the class.
  //   Here, since the next_ pointer is a the same access level as the
  //   getter/setter, the methods are not particularly useful. For interface
  //   compatibility, I would suggest moving them to the public section.
  // RESPONSE(steven): Unlike pool_element, classes are expected to extend this
  // class, which was why i thought it was best to make them private and to make
  // hp_list a friend.

  /**
   * Helper method for getting the next pointer.
   */
  Element * next() { return next_; }

  /**
   * Helper method for setting the next pointer.
   */
  void next(Element *next) { next_ = next; }

  Element *next_ {nullptr};
  void operator delete( void * ) {}
  DISALLOW_COPY_AND_ASSIGN(Element);
};

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel

#endif  // TERVEL_UTIL_MEMORY_HP_POOL_ELEMENT_H_
