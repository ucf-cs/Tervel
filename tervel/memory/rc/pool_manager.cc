#include "tervel/memory/rc/descriptor_pool.h"
#include "tervel/memory/rc/pool_manager.h"


namespace tervel {
namespace memory {
namespace rc {

DescriptorPool * PoolManager::get_pool() {
  /* TODO(carlos): the above should take an TID.
  Logic related to NULL should be handled by the calling thread
  If its tid == TID then it will allocate the object through a seperate function
  Other wise it returns a reference to the pool
   Though I am not sure about the use of this function...
   The name implies it gets a pool but not that it creates a new pool.
   So we can have two functions, init_pool and get_pool.
   We use get_pool when we are iterating trying to get elements
  */
  
  if (allocated_pools_ >= number_pools_) {
    return nullptr;
  }
  int pool_id = ++allocated_pools_;
  // TODO(carlos): why
  DescriptorPool *pool = new DescriptorPool(pool_id, this);
  pools_[pool_id].pool.reset(pool);
  return pool;
}

}  // namespace rc
}  // namespace memory
}  // namespace tervel
