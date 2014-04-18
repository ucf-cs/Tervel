#ifndef UCF_THREAD_RC_POOL_MANAGER_H_
#define UCF_THREAD_RC_POOL_MANAGER_H_

#include <atomic>
#include <memory>
#include <utility>

#include <assert.h>
#include <stdint.h>

#include "thread/system.h"

namespace ucf {
namespace thread {

class Descriptor;

namespace rc {

class DescriptorPool;
class PoolElement;

class PoolManager {
 public:
  friend class DescriptorPool;

  PoolManager(int number_pools)
      : number_pools_(number_pools)
      , allocated_pools_(0)
      , pools_(new ManagedPool[number_pools_]) {}

  /**
   * Allocates a pool for thread-local use. Semantically, the manager owns all
   * pools. Method is not thread-safe.
   */
  DescriptorPool * get_pool();

  const int number_pools_;


 private:
  struct ManagedPool {
    // TODO(carlos) use a pool object, or a pool pointer?
    std::unique_ptr<DescriptorPool> pool {nullptr};

    std::atomic<PoolElement *> safe_pool;
    std::atomic<PoolElement *> unsafe_pool;

    char padding[CACHE_LINE_SIZE - sizeof(pool) - sizeof(safe_pool) -
      sizeof(unsafe_pool)];
  };
  static_assert(sizeof(ManagedPool) == CACHE_LINE_SIZE,
      "Managed pools have to be cache aligned to prevent false sharing.");

  /**
   * Keeps track of how many pools have been allocated.
   * TODO(carlos) should this be atomic?
   */
  int allocated_pools_;

  std::unique_ptr<ManagedPool[]> pools_;
};

}  // namespace rc
}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_RC_POOL_MANAGER_H_
