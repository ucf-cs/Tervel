#ifndef UCF_THREAD_RC_POOL_ELEMENT_H_
#define UCF_THREAD_RC_POOL_ELEMENT_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include "thread/descriptor.h"
#include "thread/system.h"

namespace ucf {
namespace thread {

class Descriptor;

namespace rc {

// TODO(carlos) user should never be aware of existance of the PoolElement
// object. What's the best way to hide its definition? Could be a private class
// of DescriptorPool, but need to see how meshes with hazard pointers.
class PoolElement {
 public:
  static constexpr int BASE_TYPE = 69;
  static constexpr int DEBUG_EXPECTED_STAMP = 0xDEADBEEF;

  /**
   * All the member variables of PoolElement are stored in a struct so that the
   * left over memory for cache padding can be easily calculated.
   *
   * TODO(carlos) figure out a way to generalize this header to pool elememts
   * for hazard pointer'd pools.
   */
  struct Header {
    PoolElement *next_;
    std::atomic<uint64_t> ref_count_ {0};

#ifdef DEBUG_POOL
    int type_ {BASE_TYPE};

    std::atomic<bool> descriptor_in_use_ {false};

    std::atomic<uint64_t> allocation_count_ {1};
    std::atomic<uint64_t> free_count_ {0};

    // This stamp is checked when doing memory pool shenanigans to make sure
    // that a given descriptor actually belongs to a memory pool.
    int debug_pool_stamp_ {DEBUG_EXPECTED_STAMP};
#endif
  };

  PoolElement(PoolElement *next=nullptr) { header_.next_ = next; }

  /**
   * Returns a pointer to the associated descriptor of this element.
   */
  Descriptor * descriptor() { return reinterpret_cast<Descriptor*>(padding_); }

  /**
   * Constructs a descriptor of the given type within this pool element. Caller
   * must be careful that there's not another descriptor already in use in this
   * element, or memory will be stomped and resources might leak.
   *
   * Call cleanup_descriptor() to call the descriptor's destructor when done
   * with it.
   */
  template<typename DescrType, typename... Args>
  void init_descriptor(Args&&... args);

  /**
   * Should be called by the owner of this element when the descriptor in this
   * element is no longer needed, and it is safe to destroy it. Simply calls the
   * destructor on the internal descriptor.
   */
  void cleanup_descriptor();

  Header header_;

  /**
   * This padding includes enough room for both the descriptor associated with
   * this pool element and the cache line padding after it.
   */
  char padding_[CACHE_LINE_SIZE - sizeof(header_)];
};
static_assert(sizeof(PoolElement) == CACHE_LINE_SIZE,
    "Pool elements should be cache-aligned. Padding calculation is probably"
    " wrong.");

/**
 * If the given descriptor was allocated through a DescriptorPool, then it has
 * an associated PoolElement header. This methods returns that PoolElement.
 *
 * Use with caution as Descriptors not allocated from a pool will not have an
 * associated header, and, thus, the returned value will be to some random
 * place in memory.
 */
PoolElement * get_elem_from_descriptor(Descriptor *descr);

/**
 * TODO(carlos) what does this do? What do the arguments mean?
 */
bool watch(Descriptor *descr, std::atomic<void *> *a, void *value);

/**
 * TODO(carlos) what does this do?
 */
void unwatch(Descriptor *descr);

/**
 * TODO(carlos) what does this do?
 */
bool is_watched(Descriptor *descr);


// IMPLEMENTATIONS
// ===============

template<typename DescrType, typename... Args>
void PoolElement::init_descriptor(Args&&... args) {
  static_assert(sizeof(DescrType) <= sizeof(padding_),
      "Descriptor is too large to use in a pool element");
#ifdef DEBUG_POOL
  assert(!header_.descriptor_in_use_.load());
  header_.descriptor_in_use_.store(true);
#endif
  new(padding_) DescrType(std::forward<Args>(args)...);
}


inline void PoolElement::cleanup_descriptor() {
#ifdef POOL_DEBUG
  assert(header_.descriptor_in_use_.load());
  header_.descriptor_in_use_.store(false);
#endif
  this->descriptor()->~Descriptor();
}


inline PoolElement * get_elem_from_descriptor(Descriptor *descr) {
  PoolElement::Header *tmp = reinterpret_cast<PoolElement::Header *>(descr) - 1;
#ifdef DEBUG_POOL
  // If this fails, then the given descriptor is not part of a PoolElement. This
  // probably means the user passed in a descriptor that wasn't allocated
  // through a memory pool.
  assert(tmp->debug_pool_stamp_ == DEBUG_EXPECTED_STAMP);
#endif
  return reinterpret_cast<PoolElement *>(tmp);
}


}  // namespace rc
}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_RC_POOL_ELEMENT_H_
