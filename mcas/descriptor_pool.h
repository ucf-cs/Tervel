#ifndef MCAS_DESCRIPTOR_POOL_H_
#define MCAS_DESCRIPTOR_POOL_H_

#define DEBUG_POOL

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

// TODO(carlos) find a better place to put this, this constant should be
// configurable based on target system.
constexpr size_t CACHE_LINE_SIZE = 64;  // bytes

namespace ucf {
namespace thread {

class Descriptor;

namespace rc {

class PoolElem;

// TODO(carlos): what's this flag for?
constexpr bool NO_REUSE_MEM = false;

/**
 * Defines a pool of descriptor objects which is used to allocate descriptors
 * and to store them while they are not safe to delete.
 *
 * TODO(carlos) Is this paragraph correct? Ask Steven.
 * The pool is represented as two linked lists of descriptors: one for safe
 * elements and one for unsafe elements. The safe pool is known to only contain
 * items owned by the thread owning the DescriptorPool object, and the unsafe
 * pool contains items where other threads may still hold refrences to them
 *
 * Further, the pool object has a parent who is shared amongst other threads.
 * When a pool is to be destroyed, it sends its remaining elements to the
 * parent, relinquishing ownership of said elements. A top-level pool has a null
 * parent. At the moment, it only makes sense to have a single top-level parent
 * representing the central pool for all threads, and several local pools for
 * each thread.
 *
 * TODO(carlos) steven probably won't like this parent concept, but it's cheap
 * to implement and moves away from compile-time globals to a more composable
 * interface.
 */
class DescriptorPool {
 public:
  explicit DescriptorPool(DescriptorPool* parent=nullptr) : parent_(parent) {}

  /**
   * Releases the descriptor back to the safe pool. Caller relinquishes
   * ownership of descriptor. It's expected that the given descriptor was taken
   * from this pool to begin with.
   *
   * TODO(carlos) What are the guarantees on when the descriptor's destructor is
   * called? Does adding to the safe pool mean it's safe to call the destructor,
   * but in the unsafe pool it may not be?
   *
   * TODO(carlos) How does the caller distinguish when to use this function vs.
   * when to use the unsafe variant?
   */
  void add_to_safe(Descriptor* descr);

  /**
   * Releases the descriptor back to the unsafe pool. Caller relinquishes
   * ownership of descriptor. It's expected that the given descriptor was taken
   * from this pool to begin with.
   *
   * See notes on add_to_safe()
   */
  void add_to_unsafe(Descriptor* descr);

  /**
   * Clear all elements from the safe pool.
   */
  void free_safe();

  /**
   * Try to move elements from the unsafe pool to the safe pool.
   */
  void try_free_unsafe(bool force=false);

  /**
   * Sends all elements managed by this pool to the parent pool. Same as:
   *   send_safe_to_parent();
   *   send_unsafe_to_parent();
   * TODO(carlos): what should happen if parent_ is null?
   */
  void send_to_parent();

  /**
   * Sends the elements from the safe pool to the parent pool's safe pool.
   */
  void send_safe_to_parent();

  /**
   * Sends the elements from the unsafe pool to the parent pool's unsafe pool.
   */
  void send_unsafe_to_parent();

  /**
   * Constructs a Descriptor of the given type and returns a pool element
   * containing that descriptor. Arguments are forwarded to the constructor of
   * the given descriptor type.
   */
  template<typename DescrType, typename... Args>
  Descriptor * get_descriptor(Args&&... args);


#ifdef DEBUG_POOL
  uint64_t safe_pool_count_ {0};
  uint64_t unsafe_pool_count_ {0};
#endif

 private:
  /**
   * If the given descriptor was allocated through a DescriptorPool, then it has
   * an associated PoolElem header. This methods returns that PoolElem.
   *
   * Use with caution as Descriptors not allocated from a pool will not have an
   * associated header, and, thus, the returned value will be to some random
   * place in memory.
   */
  static PoolElem * get_elem_from_descriptor(Descriptor *descr);

  /**
   * Gets a free element from this pool. If there are no free elements to
   * retrieve from the pool, a new one is allocated if allocate_new is true, and
   * nullptr is returned if it is false.
   */
  PoolElem * get_from_pool(bool allocate_new=true);

  /**
   * Refrence to some pool that's shared between other threads. All accesses to
   * this pool have to be done in an atomic fashion.
   */
  DescriptorPool *parent_;

  // TODO(carlos) I really don't know the distinction between these. Will have
  // to ask steven to write it down.
  PoolElem *safe_pool_ {nullptr};
  PoolElem *unsafe_pool_ {nullptr};
};


// TODO(carlos) user should never be aware of existance of the PoolElem object.
// What's the best way to hide its definition? Could be a private class of
// DescriptorPool, but need to see how meshes with hazard pointers.
class PoolElem {
 public:
  static constexpr int BASE_TYPE = 69;

  /**
   * All the member variables of PoolElem are stored in a struct so that the
   * left over memory for cache padding can be easily calculated.
   *
   * TODO(carlos) figure out a way to generalize this header to pool elememts
   * for hazard pointer'd pools.
   */
  struct Header {
    std::atomic<uint64_t> ref_count_ {0};

#ifdef DEBUG_POOL
    int type_ {BASE_TYPE};

    std::atomic<bool> descriptor_in_use_ {false};

    std::atomic<uint64_t> allocation_count_ {1};
    std::atomic<uint64_t> free_count_ {0};

    // This stamp is checked when doing memory pool shenanigans to make sure
    // that a given descriptor actually belongs to a memory pool.
    const int kDebugPoolStamp = 0xDEADBEEF;
#endif
  };

  PoolElem(PoolElem *next=nullptr) { next_ = next; }

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

  PoolElem *next_;
  Header header_;

 private:
  /**
   * This padding includes enough room for both the descriptor associated with
   * this pool element and the cache line padding after it.
   */
  char padding_[CACHE_LINE_SIZE - sizeof(header_) - sizeof(next_)];
};
static_assert(sizeof(PoolElem) == CACHE_LINE_SIZE,
    "Pool elements should be cache-aligned. Padding calculation is probably"
    " wrong.");


// TEMPLATE IMPLEMENTATIONS
// ========================

template<typename DescrType, typename... Args>
Descriptor * DescriptorPool::get_descriptor(Args&&... args) {
  PoolElem *elem = this->get_from_pool();
  elem->init_descriptor<DescrType>(std::forward<Args>(args)...);
  return elem->descriptor();
}

template<typename DescrType, typename... Args>
void PoolElem::init_descriptor(Args&&... args) {
  static_assert(sizeof(DescrType) <= sizeof(padding_),
      "Descriptor is too large to use in a pool element");
#ifdef DEBUG_POOL
  assert(!header_.descriptor_in_use_.load());
  header_.descriptor_in_use_.store(true);
#endif
  new(padding_) DescrType(std::forward<Args>(args)...);
}

}  // namespace rc
}  // namespace thread
}  // namespace ucf

#endif  // MCAS_DESCRIPTOR_POOL_H_

