#ifndef TERVEL_UTIL_DESCRIPTOR_H_
#define TERVEL_UTIL_DESCRIPTOR_H_

#include <atomic>

#include <assert.h>
#include <stdint.h>

#include <tervel/util/info.h>
#include <tervel/util/util.h>

namespace tervel {
namespace util {
/**
 * This defines the Descriptor class, this class is designed to be extend
 * and be used in conjunction with primarily the RC memory pool objects.
 * Extending this class allows the developer to quickly create RC protected
 * elements.
 *
 * Classes that extend this class must implement the following functions:
 *    complete
 *    get_logical_function.
 * This allows for various algorithms and data structures to be executed
 * on overlapping regions of memory.
 *
 * For use with memory protection schemes we provide the following functions:
 *    on_watch
 *    on_is_watched
 *    on_unwatch
 * These are called by the memory protection scheme in the event more advance
 * logic is required to safely dereference of free such objects.
 *
 * If an object contains a reference to other object(s) that can only be freed
 * when it is freed then this must expressed in the objects destructor.
 */
class Descriptor {
 public:
  Descriptor() {}
  virtual ~Descriptor() {}
  /**
  * This method is implemented by each sub class and must guarantee that upon
  * return that the descriptor no longer exists at the address it was placed
  *
  * @param current the reference to this object as it is at the address,
  * @param address the location this object was read from
  */
  virtual void * complete(void *current, std::atomic<void *> *address) = 0;

  /**
   * This method is implemented by each sub class. It returns the logical value
   * of the past address.  If the associated operation is still in progress then
   * it will generally return the value that was replaced by this descriptor.
   * Otherwise it will generally return the result of the operation for the
   * specified address.
   *
   * It can only be called from the static function which protects the object
   * from being reused during the function.
   */
  virtual void * get_logical_value() = 0;

  /**
   * This method is optional to implement for each sub class.  In the event there
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
   * @return true if successful, false otherwise
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

 private:
  DISALLOW_COPY_AND_ASSIGN(Descriptor);
};

}  // namespace util
}  // namespace tervel


#endif  // TERVEL_UTIL_DESCRIPTOR_H_
