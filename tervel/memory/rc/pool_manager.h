#ifndef TERVEL_MEMORY_RC_POOL_MANAGER_H_
#define TERVEL_MEMORY_RC_POOL_MANAGER_H_

#include <atomic>
#include <memory>
#include <utility>

#include <assert.h>
#include <stdint.h>

#include "tervel/memory/system.h"
#include "tervel/util.h"

namespace tervel {
namespace memory {

class Descriptor;

namespace rc {

class DescriptorPool;
class PoolElement;

/**
 * Encapsulates a shared central pool between several thread-local pools. Idea
 * is that each thread gets a local pool to grab descriptors from, and that each
 * of these pools is managed by a single instance of this class. The thread
 * local pools can periodically release their unused elements into the shared
 * pool in this manager, or can take elements from the shared pools in this
 * manager.
 */
class PoolManager {
 public:
  friend class DescriptorPool;

  explicit PoolManager(int number_pools)
      : number_pools_(number_pools)
      // REVIEW(carlos): no such element allocated_pools_
      , allocated_pools_(0)
      , pools_(new ManagedPool[number_pools_]) {}

  /**
   * Allocates a pool for thread-local use. Semantically, the manager owns all
   * pools. Method is thread-safe iff each thread calls it with a unique pos,
   * the thread's ID satifies this
   */
  // REVIEW(carlos): Initially, I had designed the semantics of get_pool to
  //   count the number of pools it needs to allocate, and to allocate "on
  //   demand" when the function was called. (In retrospect, a name like
  //   allocate_pool would have been better). The way it's been changed is
  //   dangerous because if it's called twice with the same index, the previous
  //   pool is deleted, causing some thread to have a pointer to deleted memory
  //   and causing a memory leak of the elements inside the pool. If you want it
  //   to simply act as an accessor to a pool with a given index, then it's best
  //   if each `pool' member in the ManagedPool array was simply constructed in
  //   the constructor of this class, and that get_pool simply acted as an
  //   accessor for the element, rather than trying to allocate it.
  // REVIEW(carlos): compiler error, you have to include info.h for the forward
  //   declaration of tl_thread_info
  DescriptorPool * get_pool(int pos = tl_thread_info.thread_id);

  const int number_pools_;


 private:
  struct ManagedPool {
    // REVIEW(carlos): missing semi colon
    std::unique_ptr<DescriptorPool> pool {nullptr}
    // REVIEW(carlos): how are these linked lists freed? Destructor should
    // traverse the lists and delete each element from it for safety.
    std::atomic<PoolElement *> safe_pool {nullptr}
    std::atomic<PoolElement *> unsafe_pool {nullptr}

    char padding[CACHE_LINE_SIZE - sizeof(pool) - sizeof(safe_pool) -
      sizeof(unsafe_pool)];
  };
  static_assert(sizeof(ManagedPool) == CACHE_LINE_SIZE,
      "Managed pools have to be cache aligned to prevent false sharing.");


  std::unique_ptr<ManagedPool[]> pools_;

  DISALLOW_COPY_AND_ASSIGN(PoolManager);
};

}  // namespace rc
}  // namespace memory
}  // namespace tervel

#endif  // TERVEL_MEMORY_RC_POOL_MANAGER_H_
