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

class PoolManager {
 public:
  PoolManager(int number_pools)
      : number_pools_(number_pools)
      , pools_(new ManagedPool[number_pools_]) {}

  /**
   * Allocates a pool for thread-local use. Semantically, the manager owns all
   * pools.
   */
  DescriptorPool * get_pool();


 private:
  struct ManagedPool {
    // TODO(carlos) use a pool object, or a pool pointer?
    DescriptorPool *pool {nullptr};

    // TODO(carlos) what other things belong in here?

    char padding_[CACHE_LINE_SIZE - sizeof(pool)];
  };
  static_assert(sizeof(ManagedPool) == CACHE_LINE_SIZE,
      "Managed pools have to cache aligned to prevent false sharing.");

  int number_pools_;
  std::unique_ptr<ManagedPool[]> pools_;
};

}  // namespace rc
}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_RC_POOL_MANAGER_H_
