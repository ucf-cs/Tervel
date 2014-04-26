#ifndef TERVEL_MEMORY_DESCRIPTOR_H_
#define TERVEL_MEMORY_DESCRIPTOR_H_

#include <atomic>

#include <assert.h>
#include <stdint.h>

#include "tervel/memory/info.h"
#include "tervel/memory/rc/util_descriptor.h"
#include "tervel/util.h"


namespace tervel {
namespace memory {

namespace rc { class DescriptorPool; }
// REVIEW(carlos): There is no class with the name HPElementPool.
namespace hp { class HPElementPool; }

class Descriptor {
  // REVIEW(carlos): Trailing whitespace in class comment block
  // REVIEW(carlos): Class comments go above the opening line:
  //   /**
  //    * I am a class comment.
  //    */
  //    class MyClass {
  //      ...
  //    };
  /**
   * This defines the Descriptor class, this class is designed to be extened
   * and be used in conjunction with primariliy the RC memory pool objects.
   * Extending this class allows the developer to quickly create RC protected
   * elements.
   * 
   * Classes that extend this class must implement the following functions:
   *    complete
   *    get_logical_function.
   * This allows for various algorithms and data structures to be executed
   * on overlaping regions of memory.
   * 
   * For use with memory protection schemes we provide the following functions:
   *    on_watch
   *    on_is_watched
   *    on_unwatch
   * These are called by the memory protection scheme in the event more advance
   * logic is required to safely dereference of free such objects.
   *
   * If an object contains a reference to other object(s) that can only be freed
   * when it is freed then this must expressed in the objects descructor. 
   */
 public:
  Descriptor() {}
  virtual ~Descriptor() {}
  /**
  * This method is implmented by each sub class and must gurantee that upon
  * return that the descriptor no longer exists at the address it was placed
  *
  * @param current the reference to this object as it is at the address,
  * @param address the location this object was read from
  */

  // REVIEW(carlos): Shouldn't be a blank line between the function declaration
  //   and its comment block.
  virtual void * complete(void *current, std::atomic<void *> *address) = 0;

 private:
  /**
   * This method is implmented by each sub class. It returns the logical value
   * of the past address.  If the associted operation is still in progress then
   * it will generally return the value that was replaced by this descriptor.
   * Otherwise it will generally return the result of the operation for the
   * specified addres.
   *
   * It can only be called from the static function which protects the object
   * from being reused during the function.
   */
  virtual void * get_logical_value() = 0;

  /**
   * This method is optional to implment for each sub class.  In the event there
   * is a complex dependency between descriptor objects, where watching one
   * implies performing other actions, such as watching a parent object, a
   * developer will implement this function to encapsulate that logic
   *
   * This function is called by the static watch function It should not watch
   * itself.
   *
   * @param address The location to check.
   * @param expected The expected value for that location
   *
   * @return true if successfull, false otherwise
   */

  // REVIEW(carlos): Shouldn't be a blank line between the function declaration
  //   and its comment block.
  // REVIEW(carlos): Line is longer than 80 chars.
  virtual bool on_watch(std::atomic<void *> * /*address*/, void * /*expected*/) {
    return true;
  }

  /**
   * This method must be implemented if on_watch is implemented, and is
   * optional otherwise.  It must unwatch any object watched by on_watch.
   * It should not unwatch itself. It is called when this descriptor is
   * unwatched.
   */
  virtual void on_unwatch() {}

  /**
   * This method is optional to implement for each sub class.
   * This function must be implemented if on_watch is implemented.
   *
   * @return true iff the item is watched by another thread
   */

  // REVIEW(carlos): Shouldn't be a blank line between the function declaration
  //   and its comment block.
  virtual bool on_is_watched() { return false; }


public:
  /**
   * This method is used to get the value stored at an address that may have a
   * descriptor object their.  It handles memory protection of the objects.  It
   * also performs the read in a wait-free manner using the progress assurance
   * scheme.
   *
   * @param address the address to read
   * @return the logical value of the address
   */
  // REVIEW(carlos): The pointer star should be consistently placed within a
  //   file. In other declarations, it's with the variable name, so it should be
  //   with the variable name here:
  //     std::atomic<T> *address
  template<class T>
  static T read(std::atomic<T> * address);

  // REVIEW(carlos): Comment text should start after the line with the double
  //   star
  /** This Method determins if the passed value is a descriptor or not.
   * It does so by calling the two static is_descriptor functions of the RC and 
   * HP descriptor classes.
   *
   * @param value the value to check
   * @return true if is a descriptor
   */
  static bool is_descriptor(void *value) {
    // REVIEW(carlos): There is no `namespace RC' (capital letters)
    if (RC::is_descriptor_first(value)) {
      return true;
    }
    return false;
  }

  // REVIEW(carlos): Comment text should start after the line with the double
  //   star
  /** This method removes a descriptor from the passed address.
   * First it determineds the type of descriptor, currently only supports RC
   * Then it calls that descriptor type's watch procedure
   * Finally it calls the descriptor complete function
   *
   * @param address the address the descriptor was read from value to check
   * @param value the expected value for the address, which should be a 
   * descriptor
   * @return the current value
   *
   * TODO(steven): code this.
   */

  // REVIEW(carlos): Shouldn't be a blank line between the function declaration
  //   and its comment block.
  // REVIEW(carlos): Instead of assert(false), consider just not giving the
  //   function a body. Compiler doensn't complain if a function is declared,
  //   not defined, but never used. This causes a compile-time error if you use
  //   the function to remind you that you haven't implemented it.
  static void *remove_descriptor(std::atomic<void *> *address, void *value) {
    // REVIEW(carlos): There is no `namespace RC' (capital letters)
    if (RC::is_descriptor_first(value)) {
      assert(false);
    }
    // This assert hits in the event an unknown discriptor type wass passed in
    assert(false);
    return false;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Descriptor);
};

}  // namespace thread
}  // namespace tervel


#endif  // TERVEL_MEMORY_DESCRIPTOR_H_
