#include "thread/rc/pool_manager.h"
#include "thread/rc/descriptor_pool.h"


namespace ucf {
namespace thread {
namespace rc {

DescriptorPool * PoolManager::get_pool() {
  if (allocated_pools_ >= number_pools_) {
    return nullptr;
  }
  int pool_id = ++allocated_pools_;
  DescriptorPool *pool = new DescriptorPool(pool_id, this);
  pools_[pool_id].pool.reset(pool);
  return pool;
}

}  // namespace rc
}  // namespace thread
}  // namespace ucf
