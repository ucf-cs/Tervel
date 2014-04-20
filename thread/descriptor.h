#ifndef UCF_THREAD_DESCRIPTOR_H_
#define UCF_THREAD_DESCRIPTOR_H_

#include <atomic>

#include <assert.h>
#include <stdint.h>

#include "thread/info.h"


namespace ucf {
namespace thread {

namespace rc { class DescriptorPool; }

class Descriptor {
 public:
  Descriptor() {}
  virtual ~Descriptor() {}



   /**
   * This method is implmented by each sub class and must gurantee that upon
   * return that the descriptor no longer exists at the address it was placed
   * @param The bitmarked reference to this object, and the address it was 
   * placed at.
   */

  virtual void * complete(void *t, std::atomic<void *> *address) = 0;

  /**
   * This method is implmented by each descriptor object that can be placed into
   * the pending operation table. It must contain all the information necessary
   * to complete the associated operation
   * @param  none
   */

  virtual void help_complete() = 0;

//TODO(carlos): Below is private functions that should only be called by our
// Functions

  /**
   * This method is implmented by each sub class.
   * It returns the logical value of the past address.
   * If the associted operation is still in progress then it will generally 
   * return the value that was replaced by this descriptor. Otherwise it will
   * generally return the result of the operation for the specified addres.
   *
   * It can only be called from the static function which protects the object
   * from being reused during the function.
   *
   * @param none
   */
  virtual void * get_logical_value() = 0;

  /**
   * This method is optional to implment for each sub class.
   * In the event there is a complex dependency between descriptor objects, where
   * watching one implies performing other actions, such as watching a parent
   * object, a developer will implement this function to encapsulate that logic
   *
   * This function is called by the static watch function
   * It should not watch itself.
   * @param The address the object resides and the value of the object at that
   * address
   */

  virtual bool advance_watch(std::atomic<void *> *, void *) { return true; }

  /**
   * This method is optional to implement for each sub class.
   * This function must be implemented if advance_watch is implemented.
   * It must unwatch any object watched by advance_watch.
   * It should not unwatch itself.
   *
   * This function is called by the static unwatch function
   * @param none
   */
  virtual void advance_unwatch() {}

  /**
   * This method is optional to implement for each sub class.
   * This function must be implemented if advance_watch is implemented.
   * 
   * This function returns true/false depending on the watch status
   * @param none
   */

  virtual bool advance_is_watched() { return false; }

  /**
   * This method is optional to implement for each sub class.
   *  It should free any child objects.
   
   * This method is called by a DescriptorPool when a descriptor is deleted but
   * before the destructor is called. This way, associated descriptors can be
   * recursively freed.
   * TODO(carlos): do crazy stuff with friends of rc::DescriptorPool
   *
   * @param pool The memory pool which this is being freed into.
   */
  virtual void advance_return_to_pool(rc::DescriptorPool * /*pool*/) {}

 
};

// TODO(carlos) not sure where to put the below functions
// Lets move into RC, this way it is clear that it is an rc protected descriptor
// We can add them to HP with different bits as well. ie use 2 instead of 1

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



}  // namespace thread
}  // namespace ucf


#endif  // UCF_THREAD_DESCRIPTOR_H_
