#include "thread/rc/descriptor_pool.h"
#include "thread/rc/pool_manager.h"


namespace ucf {
namespace thread {
namespace rc {

DescriptorPool * PoolManager::get_pool() {
  // REVIEW(carlos) (╯°□°）╯︵ ┻━┻ stop using my name, imposter!
  /**
   * TODO(carlos): the above should take an TID.
   *   Logic related to NULL should be handled by the calling thread
   *   If its tid == TID then it will allocate the object through a seperate function
   *   Other wise it returns a reference to the pool
   *   Though I am not sure about the use of this function...
   *   The name implies it gets a pool but not that it creates a new pool.
   *   So we can have two functions, init_pool and get_pool.
   *   We use get_pool when we are iterating trying to get elements
   *
   *   (the-real-carlos) I wasn't too sure about it either. Figured that this
   *     would be called as an entry point to allocating a new pool. splitting
   *     things into two functions sounds fine, though I don't see the use case
   *     for grabbing a pool by id.
  */
  
  if (allocated_pools_ >= number_pools_) {
    return nullptr;
  }
  int pool_id = ++allocated_pools_;
  // REVIEW(carlos) see above comment.
  // TODO(carlos): why
  //   (the-real-carlos): taken directly from ucf_threading.h
  DescriptorPool *pool = new DescriptorPool(pool_id, this);
  pools_[pool_id].pool.reset(pool);
  return pool;
}

}  // namespace rc
}  // namespace thread
}  // namespace ucf
