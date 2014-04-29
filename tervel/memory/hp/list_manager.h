// REVIEW(carlos): include guard name should be TERVEL_MEMORY_HP_POOL_MANAGER_H_
#ifndef UCF_THREAD_HP_POOL_MANAGER_H_
#define UCF_THREAD_HP_POOL_MANAGER_H_

#include <atomic>
#include <memory>
#include <utility>

#include <assert.h>
#include <stdint.h>

// REVIEW(carlos): should be tervel/memory/system.h"
#include "thread/system.h"

// REVIEW(carlos): should be `namespace tervel'
namespace ucf {
namespace memory {
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

  // REVIEW(carlos): space ob both sides on colon
  ListManager(): pool_(new ManagedList) {}


 private:
  // REVIEW(carlos): misnomer. this isn't a list, it's an element of a list.
  struct ManagedList {
    // REVIEW(carlos): missing semi colon
    std::atomic<HPElement *> to_free_list {nullptr}

    // REVIEW(carlos): below is a review comment from a previous iteration.
    //   Please strip said comments before sending off to new review. Especially
    //   if you've already addressed the issue.
    // REVIEW(carlos) no such member `pool' Will cause an error.
    char padding[CACHE_LINE_SIZE - sizeof(to_free_list)];
  };
  static_assert(sizeof(ManagedList) == CACHE_LINE_SIZE,
      "Managed lists have to be cache aligned to prevent false sharing.");

  // REVIEW(carlos): I don't see the need for keeping this as a unique_ptr if
  //   it's the head of a linked list. In rc, the reason was that the analogous
  //   member was an array, and the deletion of the manager would cause the
  //   array to be traversed and the destructor called on the contained
  //   descriptor pools. If you're keeping the pool_ here as a linked list of
  //   ManagedList's, then you'll need to manually traverse the list and delete
  //   the contained elements at destruction time.
  std::unique_ptr<ManagedList> pool_;
};

}  // namespace hp
// REVIEW(carlos): should be namespace memory
}  // namespace thread
// REVIEW(carlos): should be namespace tervel
}  // namespace ucf

// REVIEW(carlos): include guard name should be TERVEL_MEMORY_HP_POOL_MANAGER_H_
#endif  // UCF_THREAD_HP_POOL_MANAGER_H_
