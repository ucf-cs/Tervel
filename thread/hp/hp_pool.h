#ifndef UCF_THREAD_HP_DESCRIPTOR_POOL_H_
#define UCF_THREAD_HP_DESCRIPTOR_POOL_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>

#include "thread/descriptor.h"
#include "thread/info.h"
#include "thread/hp/pool_element.h"
#include "thread/hp/pool_manager.h"
#include "thread/system.h"

namespace ucf {
namespace thread {
namespace hp {

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
class HPPool {
 public:
  explicit HPPool(PoolManager *manager)
      : manager_(manager) {
  }
  ~HPPool() { this->send_to_manager(); }



 private:
  // -------------------------
  // FOR DEALING WITH MANAGERS
  // -------------------------

  /**
   * Sends all elements managed by this pool to the parent pool.
   */
  void send_to_manager();




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
  HPElement *unsafe_pool_ {nullptr}

#ifdef DEBUG_POOL
  // TODO(carlos) what, exactly, do these keep track of?
  uint64_t unsafe_pool_count_ {0}
#endif

  DISALLOW_COPY_AND_ASSIGN(HPPool);
};

}  // namespace hp
}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_HP_DESCRIPTOR_POOL_H_

