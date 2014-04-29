// REVIEW(carlos): include guard name should be TERVEL_MEMORY_HP_POOL_MANAGER_H_
#ifndef TERVEL_UTIL_MEMORY_HP_LIST_MANAGER_H_
#define TERVEL_UTIL_MEMORY_HP_LIST_MANAGER_H_

#include <atomic>
#include <memory>
#include <utility>

#include <assert.h>
#include <stdint.h>

#include "tervel/util/system.h"
#include "tervel/util/memory/hp/hp_element.h"
#include "tervel/util/memory/hp/hp_list.h"

namespace tervel {
namespace util {
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

  ListManager() : pool_() {}

  ~ListManager() {
    // TODO(steven): destroy pool by freeing each HPElement
  }


 private:
  std::atomic<HPElement *> to_free_list_ {nullptr};
};

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel
#endif  // TERVEL_UTIL_MEMORY_HP_LIST_MANAGER_H_
