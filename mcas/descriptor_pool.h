#ifndef MCAS_DESCRIPTOR_POOL_H_
#define MCAS_DESCRIPTOR_POOL_H_

#define DEBUG_POOL

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>

#include "pool_element.h"

namespace ucf {
namespace thread {

class Descriptor;

namespace rc {

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
   * an associated PoolElement header. This methods returns that PoolElement.
   *
   * Use with caution as Descriptors not allocated from a pool will not have an
   * associated header, and, thus, the returned value will be to some random
   * place in memory.
   */
  static PoolElement * get_elem_from_descriptor(Descriptor *descr);

  /**
   * Gets a free element from this pool. If there are no free elements to
   * retrieve from the pool, a new one is allocated if allocate_new is true, and
   * nullptr is returned if it is false.
   */
  PoolElement * get_from_pool(bool allocate_new=true);

  /**
   * Refrence to some pool that's shared between other threads. All accesses to
   * this pool have to be done in an atomic fashion.
   */
  DescriptorPool *parent_;

  // TODO(carlos) I really don't know the distinction between these. Will have
  // to ask steven to write it down.
  PoolElement *safe_pool_ {nullptr};
  PoolElement *unsafe_pool_ {nullptr};
};


// TEMPLATE IMPLEMENTATIONS
// ========================

template<typename DescrType, typename... Args>
Descriptor * DescriptorPool::get_descriptor(Args&&... args) {
  PoolElement *elem = this->get_from_pool();
  elem->init_descriptor<DescrType>(std::forward<Args>(args)...);
  return elem->descriptor();
}

}  // namespace rc
}  // namespace thread
}  // namespace ucf

#endif  // MCAS_DESCRIPTOR_POOL_H_

