#ifndef TERVEL_MEMORY_DESCRIPTOR_H_
#define TERVEL_MEMORY_DESCRIPTOR_H_

#include <atomic>

#include <assert.h>
#include <stdint.h>

#include "tervel/memory/info.h"


namespace tervel {
namespace thread {

namespace rc { class DescriptorPool; }
namespace hp { class HPElementPool; }

class Descriptor {
 public:
  Descriptor() {}
  virtual ~Descriptor() {}
   // REVIEW(carlos): excesive whitespace



   /**
   * This method is implmented by each sub class and must gurantee that upon
   * return that the descriptor no longer exists at the address it was placed
   *
   * REVIEW(carlos) param name? Other parameter? There shouldn't be a space
   *   between the function and the comment block. The stars on the comment
   *   aren't aligned.
   * @param The bitmarked reference to this object, and the address it was
   *   placed at.
   */

  virtual void * complete(void *t, std::atomic<void *> *address) = 0;

  /**
   * This method is implmented by each descriptor object that can be placed into
   * the pending operation table. It must contain all the information necessary
   * to complete the associated operation
   * REVIEW(carlos) should be a blank line between descriptions and
   *   documentation directives
   * REVIEW(carlos) don't bother putting none if there aren't any parameters
   * @param  none 
   */

  virtual void help_complete() = 0;

// TODO(carlos): Below are private functions that should only be called by our
//   Functions. Make below functions protected and add a list of friend classes
//   which access them.
//
// REVIEW(carlos) keep comments at the same indentation level as surrounding
//   code.

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
   * TODO(carlos) what are the parameter names and uses? below param declaration
   * doesn't give a name.
   *
   * @param The address the object resides and the value of the object at that
   * address
   *
   * @return TODO(carlos)
   */

  virtual bool advance_watch(std::atomic<void *> * /* address */,
      void * /* TODO(carlos) param name */) {
    return true;
  }

  /**
   * This method must be implemented if advance_watch is implemented, and is
   * optional otherwise.  It must unwatch any object watched by advance_watch.
   * It should not unwatch itself. It is called when this descriptor is
   * unwatched.
   */
  virtual void advance_unwatch() {}

  /**
   * This method is optional to implement for each sub class.
   * This function must be implemented if advance_watch is implemented.
   *
   * @return true/false depending on the watch status. TODO(carlos)
   *   no idea what this means.
   */

  virtual bool advance_is_watched() { return false; }

  /**
   * This method is optional. It should free any child objects.  This method is
   * called by a DescriptorPool when a descriptor is deleted but before the
   * destructor is called. This way, associated descriptors can be recursively
   * freed.
   *
   * @param pool The memory pool which this is being freed into.
   */
  virtual void advance_return_to_pool(rc::DescriptorPool * /*pool*/) {}


  /**
   * This method is used to get the value stored at an address that may have a
   * descriptor object their.  It handles memory protection of the objects.  It
   * also performs the read in a wait-free manner using the progress assurance
   * scheme.
   *
   * TODO(carlos) should this be a member of ProgressAssurance since it needs
   *   the scheme to be wait-free?
   * REVIEW(carlos): I don't like havine static methods in classes in general.
   *   It would be better if it was just a regular function.
   *
   * @param the address to read
   */
  template<class T>
  static T read(std::atomic<T> * address);
};

// REVIEW(carlos) If you feel this is the correct way forward, just implement
//   it. If you're not sure, the comment is fine, but do something to mark it as
//   a continuation of the TODO.
//
// TODO(carlos) not sure where to put the below functions
//   (steven) Lets move into RC, this way it is clear that it is an rc protected
//   descriptor We can add them to HP with different bits as well. ie use 2
//   instead of 1

inline void * mark(Descriptor *descr) {
  return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(descr) | 0x1L);
}

inline Descriptor * unmark(void *descr) {
  return reinterpret_cast<Descriptor *>(
      reinterpret_cast<uintptr_t>(descr) & ~0x1L);
}

inline bool is_descriptor(void *descr) {
  return (0x1L == (reinterpret_cast<uintptr_t>(descr) & 0x1L));
}
// REVIEW(carlos) excessive whitespace



}  // namespace thread
}  // namespace tervel


#endif  // TERVEL_MEMORY_DESCRIPTOR_H_
