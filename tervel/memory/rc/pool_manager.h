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
      , allocated_pools_(0)
      , pools_(new ManagedPool[number_pools_]) {}

  /**
   * Allocates a pool for thread-local use. Semantically, the manager owns all
   * pools. Method is thread-safe iff each thread calls it with a unique pos,
   * the thread's ID satifies this
   */
  DescriptorPool * get_pool(int pos = tl_thread_info.thread_id);

  const int number_pools_;


 private:
  struct ManagedPool {
    std::unique_ptr<DescriptorPool> pool {nullptr}
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
