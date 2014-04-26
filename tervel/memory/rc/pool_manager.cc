#include "tervel/memory/rc/descriptor_pool.h"
#include "tervel/memory/rc/pool_manager.h"


namespace tervel {
namespace memory {
namespace rc {

DescriptorPool * PoolManager::get_pool(int pos) {
  DescriptorPool *pool = new DescriptorPool(pos, this);
  pools_[pos].pool.reset(pool);
  return pool;
}

}  // namespace rc
}  // namespace memory
}  // namespace tervel
