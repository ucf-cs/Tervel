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
#include <tervel/util/memory/rc/descriptor_util.h>

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

  ~PoolManager() {
    for (size_t i = 0; i < number_pools_; i++) {
      // Free Unsafe Pools first.
      PoolElement *lst = pools_[i].unsafe_pool.exchange(nullptr);
      while (lst != nullptr) {
        PoolElement *next = lst->next();

        tervel::util::Descriptor *temp_descr;
        temp_descr = lst->descriptor();

        assert(!util::memory::rc::is_watched(temp_descr) &&
          " memory is not being unwatched...");

        delete lst;
        lst = next;
      }

      lst = pools_[i].safe_pool.exchange(nullptr);
      while (lst != nullptr) {
        PoolElement *next = lst->next();

        tervel::util::Descriptor *temp_descr;
        temp_descr = lst->descriptor();

        assert(!util::memory::rc::is_watched(temp_descr) &&
          " memory is not being unwatched and it was in the safe list!...");

        delete lst;
        lst = next;
      }

    }
  }

  /**
   * Allocates a pool for thread-local use.
   */
  DescriptorPool * allocate_pool(const uint64_t pid);

  void get_safe_elements(PoolElement ** pool, int *count, int min_elem) {
    assert(*pool == nullptr);
    assert(*count == 0);

    for (size_t i = 0; i < number_pools_; i++) {
      PoolElement *temp = pools_[i].safe_pool.load();
      if (temp != nullptr) {
        PoolElement *temp = pools_[i].safe_pool.exchange(nullptr);

        if (temp == nullptr) {
          continue;
        }

        PoolElement *tail = temp;
        while (tail->next() != nullptr) {
          (*count)++;
          tail = tail->next();
        }
        tail->next(*pool);
        *pool = tail;

        if (*count >= min_elem) {
          break;
        }
      }
    }  // for
  }



  void add_safe_elements(uint64_t pid, PoolElement *pool, PoolElement *pool_end = nullptr) {
    assert(pool != nullptr);

    PoolElement * temp = pools_[pid].safe_pool.load();
    if (temp != nullptr) {
      temp = pools_[pid].safe_pool.exchange(nullptr);
      if (temp != nullptr) {
        if (pool_end == nullptr) {
          pool_end = pool;
          while (pool_end->next() != nullptr) {
            pool_end = pool_end->next();
          }
        }
        assert(pool_end != nullptr);
        pool_end->next(temp);
      }
    }
    assert(pools_[pid].safe_pool.load() == nullptr);
    pools_[pid].safe_pool.store(pool);
    pool = nullptr;
  }

  void add_unsafe_elements(uint64_t pid, PoolElement *pool) {
    assert(pool != nullptr);
    assert(pools_[pid].unsafe_pool.load() == nullptr && " This should be null, this function is only called inside a destructor...pids being reused?");

    pools_[pid].unsafe_pool.store(pool);
  }

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
