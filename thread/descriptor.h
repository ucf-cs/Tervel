#ifndef MCAS_DESCRIPTOR_H_
#define MCAS_DESCRIPTOR_H_

#include <atomic>

#include <assert.h>
#include <stdint.h>


namespace ucf {
namespace thread {

namespace rc { class DescriptorPool; }

class Descriptor {
 public:
  Descriptor() {}
  virtual ~Descriptor() {}

  // TODO(carlos): these belong in the descriptor pool class
  virtual void unsafeFree() = 0;
  virtual void safeFree() = 0;

  // TODO(carlos) For all methods under this comment, we need some method
  // comment explaining the semantics of the call and what the arguments are.
  // Would do it myself, but steven's the only one who knows what any of this
  // does.

  virtual void * complete(void *, std::atomic<void *> *address) = 0;

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
  virtual void advance_return_to_pool(rc::DescriptorPool * pool);

  static uintptr_t mark(Descriptor *descr) {
    return reinterpret_cast<uintptr_t>(descr) | 0x1L;
  }

  static Descriptor * unmark(uintptr_t descr) {
    return reinterpret_cast<Descriptor *> (descr & ~0x1L);
  }

  static bool is_descriptor(uintptr_t descr) {
    return (0x1L == (descr & 0x1L));
  }

  template<class T>
  static T remove(T t, std::atomic<T> *address);

};


// IMPLEMENTATIONS
// ===============
template<class T>
T Descriptor::remove(T, std::atomic<T> *) {
  // TODO(carlos) migrate code from ucf_threading. trouble w/ use of globals
  // (which I would preffer to eliminate)
}

inline void Descriptor::advance_return_to_pool(rc::DescriptorPool *) {}


}  // namespace thread
}  // namespace ucf


#endif  // MCAS_DESCRIPTOR_H_
