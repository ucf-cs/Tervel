#ifndef UCF_THREAD_HP_POOL_MANAGER_H_
#define UCF_THREAD_HP_POOL_MANAGER_H_

#include <atomic>
#include <memory>
#include <utility>

#include <assert.h>
#include <stdint.h>

#include "thread/system.h"

namespace ucf {
namespace thread {
namespace hp {

class HPList;
class HPElement;

/**
 * Encapsulates a shared central 'to free list' between several thread-local lists.
 * When a thread is destroyed it will send its unfreeable items to this list,
 * which is freed by the user.
 */
class ListManager {
 public:
  friend class HPList;

  ListManager(): pool_(new ManagedList) {}


 private:
  struct ManagedList {
    std::atomic<HPElement *> to_free_list {nullptr}

    // REVIEW(carlos) no such member `pool' Will cause an error.
    char padding[CACHE_LINE_SIZE - sizeof(to_free_list)];
  };
  static_assert(sizeof(ManagedList) == CACHE_LINE_SIZE,
      "Managed lists have to be cache aligned to prevent false sharing.");

  std::unique_ptr<ManagedList> pool_;
};

}  // namespace hp
}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_HP_POOL_MANAGER_H_
