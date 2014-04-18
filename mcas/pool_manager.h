#ifndef MCAS_POOL_MANAGER_H_
#define MCAS_POOL_MANAGER_H_

#include <atomic>
#include <memory>
#include <utility>

#include <assert.h>
#include <stdint.h>

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
  };

  int number_pools_;
  std::unique_ptr<ManagedPool[]> pools_;
};

}  // namespace rc
}  // namespace thread
}  // namespace ucf

#endif  // MCAS_POOL_MANAGER_H_
