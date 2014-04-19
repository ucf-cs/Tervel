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

  // TODO(carlos) For all methods under this comment, we need some method
  // comment explaining the semantics of the call and what the arguments are.
  // Would do it myself, but steven's the only one who knows what any of this
  // does.

  virtual void * complete(void *t, std::atomic<void *> *address) = 0;

  virtual void help_complete() = 0;

  virtual void * get_logical_value(void *t, std::atomic<void *> *address) = 0;

  virtual bool advance_watch(std::atomic<void *> *, void *) { return true; }

  virtual void advance_unwatch() {}

  virtual bool advance_is_watched() { return false; }

  /**
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
