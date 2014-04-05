#ifndef MCAS_DESCRIPTOR_H_
#define MCAS_DESCRIPTOR_H_

#include <atomic>

#include <assert.h>
#include <stdint.h>


namespace ucf {
namespace thread {

class Descriptor {
 public:
  Descriptor() {}
  virtual ~Descriptor() {}

  virtual void unsafeFree();
  virtual void safeFree();

  // TODO(carlos) For all methods under this comment, we need some method
  // comment explaining the semantics of the call and what the arguments are.
  // Would do it myself, but steven's the only one who knows what any of this
  // does.

  virtual void * complete(void *, std::atomic<void *> *address) {
    assert(false);
    return nullptr;
  };

  virtual void help_complete() { assert(false); }

  virtual void * get_logical_value(void *t, std::atomic<void *> *address) {
    assert(false);
    return nullptr;
  };

  bool advance_watch(std::atomic<void *> *address, void *p) { return true; }
  void advance_unwatch() {}
  bool advance_is_watched() { return false; }

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


}  // namespace thread
}  // namespace ucf


#endif  // MCAS_DESCRIPTOR_H_
