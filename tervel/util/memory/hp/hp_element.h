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
#ifndef TERVEL_UTIL_MEMORY_HP_POOL_ELEMENT_H_
#define TERVEL_UTIL_MEMORY_HP_POOL_ELEMENT_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/memory/hp/hp_list.h>

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
  virtual ~Element() {}

  /**
   * This function is used to free a hazard pointer protected object if it is
   * safe to do so OR add it to a list to be freed later.
   * It also calls 'try_to_free_Elements' in an attempt to free previously
   * unfreeable objects.
   *
   * @param no_check if true then the object is imeditly deleted
   * @param element_list the list to append the object to until it is safe
   */
  void safe_delete(bool no_check = false,
      ElementList * const element_list = tervel::tl_thread_info->get_hp_element_list()) {
    #ifdef TERVEL_MEM_HP_NO_FREE
      return;
    #endif

    if (no_check) {
      delete this;
    } else {
      element_list->add_to_unsafe(this);
    }
    element_list->try_to_free_elements();
  }

  /**
   * This function is used to achieve a strong watch on an Element.
   * Classes wishing to express this should override this function.
   *
   * @param address the expected was load from
   * @param expected the last known value of address
   * @return whether or not the element was succefully watched.
   */
  virtual bool on_watch(std::atomic<void *> *address, void *expected) {
    return true;
  }

  /**
   * This function is used to check a strong watch on an Element.
   * Classes wishing to express this should override this function.
   *
   * @return whether or not the element is watched.
   */
  virtual bool on_is_watched() {return false;}
  /**
   * This function is used to remove a strong watch on an Element.
   * Classes wishing to express this should override this function.
   */
  virtual void on_unwatch() {}


 private:
  /**
   * Helper method for getting the next pointer.
   */
  Element * next() { return next_; }

  /**
   * Helper method for setting the next pointer.
   */
  void next(Element *next) { next_ = next; }

  Element *next_ {nullptr};
  // void operator delete( void * ) {}
  friend ListManager;
  friend ElementList;
  DISALLOW_COPY_AND_ASSIGN(Element);
};

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel

#endif  // TERVEL_UTIL_MEMORY_HP_POOL_ELEMENT_H_
