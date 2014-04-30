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

class ElementList;
class Element;

/**
 * Encapsulates a shared central 'to free list' between several thread-local lists.
 * When a thread is destroyed it will send its unfreeable items to this list,
 * which is freed by the user.
 */
class ListManager {
 public:
  friend class ElementList;

  ListManager(int num_pools) : pool_() {}

  ~ListManager() {
    // TODO(steven): destroy pool by freeing each Element
  }


 private:
  std::atomic<Element *> to_free_list_ {nullptr};
};

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel
#endif  // TERVEL_UTIL_MEMORY_HP_LIST_MANAGER_H_
