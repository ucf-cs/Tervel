#ifndef TERVEL_MEMORY_HP_DESCRIPTOR_POOL_H_
#define TERVEL_MEMORY_HP_DESCRIPTOR_POOL_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>

#include "tervel/memory/descriptor.h"
#include "tervel/memory/info.h"
#include "tervel/memory/hp/pool_element.h"
#include "tervel/memory/hp/pool_manager.h"
#include "tervel/memory/system.h"

namespace tervel {
namespace memory {
// REVIEW(carlos): excess vertical whitespace



namespace hp {
// REVIEW(carlos): Put blank line after block of namespaces
class HPElement;

class PoolManager;

/**
 * If true, then HPPool shouldn't free old pool elements when being
 * asked, even if it's safe to fo so. Instead, elements should be stock-piled
 * and left untouched when they're returned to the pool. This allows the user to
 * view associations. Entirely for debug purposes.
 *
 * TODO(carlos): move this to member var
 */
constexpr bool NO_REUSE_MEM = false;

/**
 * Defines a pool of objects which are stored until they are safe to be freed.
 *
 * The pool is represented as a linked lists of descriptors
 *
 * Further, the pool object has a parent who is shared amongst other threads.
 * When a pool is to be destroyed, it sends its remaining elements to the
 * parent, relinquishing ownership of said elements. A top-level pool has a null
 * parent. At the moment, it only makes sense to have a single top-level parent
 * representing the central pool for all threads, and several local pools for
 * each thread.
 */
// REVIEW(carlos): HP prefix to Pool is redundant given that the namespace name
//   is hp
class HPPool {
 public:
  // REVIEW(carlos): Use spiky lemon {} for empty function body
  // REVIEW(carlos): Can put initializer list on same line as constructor
  explicit HPPool(PoolManager *manager)
      : manager_(manager) {
  }
  ~HPPool() { this->send_to_manager(); }
  // REVIEW(carlos): There are no public methods? rc::DescriptorPool had a
  //   method for allocating an object managed by the pool, and for freeing it.
  //   I would expect this to be the same. How does one get an hp'd object,
  //   anyway, if there're no public methods to call to get one?



 private:
  // -------------------------
  // FOR DEALING WITH MANAGERS
  // -------------------------

  /**
   * Sends all elements managed by this pool to the parent pool.
   */
  void send_to_manager();
  // REVIEW(carlos): excess vertical whitespace




  // --------------------------------
  // DEALS WITH UNSAFE LIST
  // --------------------------------
  /**
   * Releases the descriptor back to the unsafe pool. Caller relinquishes
   * ownership of descriptor. It's expected that the given descriptor was taken
   * from this pool to begin with.
   *
   * See notes on add_to_safe()
   */
  void add_to_unsafe(Descriptor* descr);

  /**
   * Try to move elements from the unsafe pool to the safe pool.
   */
  void try_clear_unsafe_pool(bool dont_check = false);


  // -------
  // MEMBERS
  // -------

  /**
   * This pool's manager.
   */
  PoolManager *manager_;

  /**
   * A linked list of pool elements. Elements get released to this pool when
   * they're no longer needed, but some threads may still try to access the
   * descriptor in the element. After some time has passed, items generally move
   * from this pool to the safe_pool_
   */
  // REVIEW(carlos): If there's only an unsafe pool, then what's the point in
  //   declaring it unsafe in all functions? The safe/unsafe duality in
  //   rc::DescriptorPool is entirely private to the class and not part of the
  //   external interface, so there's no need to mimic it. I'd say it's best
  //   just to note that all items managed by an hp pool are unsafe, and leave
  //   it at that.
  HPElement *unsafe_pool_ {nullptr}

#ifdef DEBUG_POOL
  // TODO(carlos) what, exactly, do these keep track of?
  uint64_t unsafe_pool_count_ {0}
#endif
};



}  // namespace hp
}  // namespace memory
}  // namespace tervel

#endif  // TERVEL_MEMORY_HP_DESCRIPTOR_POOL_H_

