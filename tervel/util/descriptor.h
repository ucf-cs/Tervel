#ifndef TERVEL_UTIL_DESCRIPTOR_H_
#define TERVEL_UTIL_DESCRIPTOR_H_

#include <atomic>

#include <assert.h>
#include <stdint.h>

#include "tervel/util/info.h"
#include "tervel/util/memory/rc/descriptor_util.h"
#include "tervel/util/util.h"


namespace tervel {
namespace util {
class Descriptor {
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
  ~Descriptor() {}
  /**
  * This method is implmented by each sub class and must gurantee that upon
  * return that the descriptor no longer exists at the address it was placed
  *
  * @param current the reference to this object as it is at the address,
  * @param address the location this object was read from
  */
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
  virtual bool on_watch(std::atomic<void *>* /*address*/, void * /*expected*/) {
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
  template<class T>
  static T read(std::atomic<T> *address);

  /**
   * This Method determins if the passed value is a descriptor or not.
   * It does so by calling the two static is_descriptor functions of the RC and 
   * HP descriptor classes.
   *
   * @param value the value to check
   * @return true if is a descriptor
   */
  static bool is_descriptor(void *value) {
    if (memory::rc::is_descriptor_first(value)) {
      return true;
    }
    return false;
  }

  /**
   * This method removes a descriptor from the passed address.
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
  static void * remove_descriptor(std::atomic<void *> *address, void *value) {
    if (memory::rc::is_descriptor_first(value)) {
      assert(false);
    }
    // This assert hits in the event an unknown discriptor type wass passed in
    assert(false);
    return nullptr;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Descriptor);
};

}  // namespace util
}  // namespace tervel


#endif  // TERVEL_UTIL_DESCRIPTOR_H_
