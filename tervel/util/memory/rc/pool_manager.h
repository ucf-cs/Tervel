#ifndef TERVEL_MEMORY_RC_POOL_MANAGER_H_
#define TERVEL_MEMORY_RC_POOL_MANAGER_H_

#include <atomic>
#include <memory>
#include <utility>

#include <assert.h>
#include <stdint.h>

#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/system.h>
#include <tervel/util/descriptor.h>

namespace tervel {
namespace util {
namespace memory {
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

  explicit PoolManager(size_t number_pools)
      : number_pools_(number_pools)
      , pools_(new ManagedPool[number_pools]) {}

  ~PoolManager();

  /**
   * Allocates a pool for thread-local use.
   */
  DescriptorPool * allocate_pool(const uint64_t pid);

  void get_safe_elements(PoolElement **pool, uint64_t *count, uint64_t min_elem);
  void add_safe_elements(uint64_t pid, PoolElement *pool, PoolElement *pool_end = nullptr);
  void add_unsafe_elements(uint64_t pid, PoolElement *pool);


  const size_t number_pools_;

 private:
  struct ManagedPool {
    std::atomic<PoolElement *> safe_pool {nullptr};
    std::atomic<PoolElement *> unsafe_pool {nullptr};

    char padding[CACHE_LINE_SIZE  - sizeof(safe_pool) - sizeof(unsafe_pool)];
  };
  static_assert(sizeof(ManagedPool) == CACHE_LINE_SIZE,
      "Managed pools have to be cache aligned to prevent false sharing.");

  std::unique_ptr<ManagedPool[]> pools_;

  DISALLOW_COPY_AND_ASSIGN(PoolManager);
};

}  // namespace rc
}  // namespace memory
}  // namespace util
}  // namespace tervel

#endif  // TERVEL_MEMORY_RC_POOL_MANAGER_H_
