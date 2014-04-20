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

  // TODO(carlos): I think number_pools should not be passed, but rather  we use
  // the numb_threads found in the shared info to set the number of pools
  // plus pools are accessed by thread id, which must be <= num_threads.
  PoolManager(int number_pools)
      : number_pools_(number_pools)
      , allocated_pools_(0)
      , pools_(new ManagedPool[number_pools_]) {}

  /**
   * Allocates a pool for thread-local use. Semantically, the manager owns all
   * pools. Method is not thread-safe.
   * TODO(carlos): should be able to specify pool by thread id (tid).
   */
  DescriptorPool * get_pool();

  const int number_pools_;


 private:
  struct ManagedPool {
    // TODO(carlos) use a pool object, or a pool pointer?
    // TODO(carlos) Below item should not be here, I think it is for HP objects, 
    // but this is in rc name space.
    std::unique_ptr<DescriptorPool> pool {nullptr};

    std::atomic<PoolElement *> safe_pool {nullptr};
    std::atomic<PoolElement *> unsafe_pool {nullptr};

    char padding[CACHE_LINE_SIZE - sizeof(pool) - sizeof(safe_pool) -
      sizeof(unsafe_pool)];
  };
  static_assert(sizeof(ManagedPool) == CACHE_LINE_SIZE,
      "Managed pools have to be cache aligned to prevent false sharing.");

  /**
   * Keeps track of how many pools have been allocated.
   * TODO(carlos) should this be atomic?
   * TODO(carlos) No, this is not needed.
   * 
   */
  int allocated_pools_;

  std::unique_ptr<ManagedPool[]> pools_;
};

}  // namespace rc
}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_RC_POOL_MANAGER_H_
