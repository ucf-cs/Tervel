#include "descriptor_pool.h"

namespace ucf {
namespace thread {
namespace rc {


void DescriptorPool::add_to_safe(Descriptor *descr) {
  PoolElement *p = DescriptorPool::get_elem_from_descriptor(descr);
  p->next_ = safe_pool_;
  safe_pool_ = p;

  // TODO(carlos): make sure that this is the only place stuff is added to the
  // safe list from.
  p->descriptor()->advance_return_to_pool(this);
  p->cleanup_descriptor();

#ifdef DEBUG_POOL
  p->header_.free_count_.fetch_add(1);
  assert(p->header_.free_count_.load() == p->header_.allocation_count_.load());
  safe_pool_count_++;
#endif
}

PoolElement * DescriptorPool::get_elem_from_descriptor(Descriptor *descr) {
  PoolElement *tmp = reinterpret_cast<PoolElement *>(descr) - 1;
#ifdef DEBUG_POOL
  // If this fails, then the given descriptor is not part of a PoolElement. This
  // probably means the user passed in a descriptor that wasn't allocated
  // through a memory pool.
  assert(tmp->header_.debug_pool_stamp_ == DEBUG_EXPECTED_STAMP);
#endif
  return tmp;
}

PoolElement * DescriptorPool::get_from_pool(bool allocate_new) {
  // move any objects from the unsafe list to the safe list so that it's more
  // likely the safe list has something to take from.
  this->try_free_unsafe();

  PoolElement *ret {nullptr};

  // safe pool has something in it. pop the next item from the head of the list.
  if (!NO_REUSE_MEM && safe_pool_ != nullptr) {
    ret = safe_pool_;
    safe_pool_ = safe_pool_->next_;
    ret->next_ = nullptr;

#ifdef DEBUG_POOL
    // update counters to denote that an item was taken from the pool
    assert(ret->header_.free_count_.load() ==
        ret->header_.allocation_count_.load());
    ret->header_.allocation_count_.fetch_add(1);
    safe_pool_count_ -= 1;
#endif
  }

  // allocate a new element if needed
  if (ret == nullptr && allocate_new) {
    ret = new PoolElement();
  }

  return ret;
}


}  // namespace rc
}  // namespace thread
}  // namespace ucf
