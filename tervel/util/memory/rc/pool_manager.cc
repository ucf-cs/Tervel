#include "tervel/util/memory/rc/descriptor_pool.h"
#include "tervel/util/memory/rc/pool_manager.h"


namespace tervel {
namespace util {
namespace memory {
namespace rc {

DescriptorPool * PoolManager::allocate_pool(const uint64_t pid) {
  DescriptorPool *pool = new DescriptorPool(this, pid);
  return pool;
}


}  // namespace rc
}  // namespace memory
}  // namespace util
}  // namespace tervel
