#include "tervel/memory/rc/descriptor_pool.h"

namespace tervel {
namespace memory {
namespace rc {

void DescriptorPool::free_descriptor(Descriptor *descr, bool dont_check,
                , DescriptorPool *pool /*= tl_thread_info.descriptor_pool*/) {
  if (!dont_check && is_watched(descr)) {
    pool->add_to_unsafe(descr);
  } else {
    pool->add_to_safe(descr);
  }
}


void DescriptorPool::reserve(int num_descriptors) {
  for (int i = 0; i < num_descriptors; ++i) {
    PoolElement *elem = new PoolElement();
    elem->next(safe_pool_);
    safe_pool_ = elem;
  }
}


PoolElement * DescriptorPool::get_from_pool(bool allocate_new) {
  // move any objects from the unsafe list to the safe list so that it's more
  // likely the safe list has something to take from.
  this->try_clear_unsafe_pool();

  PoolElement *ret {nullptr}

  // safe pool has something in it. pop the next item from the head of the list.
  if (!NO_REUSE_MEM && safe_pool_ != nullptr) {
    ret = safe_pool_;
    safe_pool_ = safe_pool_->next();
    ret->next(nullptr);

#ifdef DEBUG_POOL
    // update counters to denote that an item was taken from the pool
    assert(ret->header().free_count.load() ==
        ret->header().allocation_count.load());
    ret->header().allocation_count.fetch_add(1);
#endif
    
    safe_pool_count_ --;
  }

  // allocate a new element if needed
  if (ret == nullptr && allocate_new) {
    ret = new PoolElement();
  }

  return ret;
}


void DescriptorPool::send_to_manager() {
  this->send_safe_to_manager();
  this->send_unsafe_to_manager();
}


namespace {

/**
 * Generic logic for clearing a pool and sending its elements to another,
 * shared pool. Parameters are passed by pointer because this function changes
 * their values.
 */
void clear_pool(PoolElement **local_pool,
    std::atomic<PoolElement *> *manager_pool) {
  if (local_pool != nullptr) {
    PoolElement *p1 = *local_pool;
    PoolElement *p2 = p1->next();
    while (p2 != nullptr) {
      p1 = p2;
      p2 = p1->next();
    }

    // p1->pool_next's value is updated to the current value
    // after a failed cas. (pass by reference fun)
    p1->next(manager_pool->load());
    while (!manager_pool->
        compare_exchange_strong(p1->header().next, *local_pool)) {}
  }
  *local_pool = nullptr;
}

}  // namespace


void DescriptorPool::send_safe_to_manager() {
  clear_pool(&safe_pool_, &this->manager_safe_pool());
}


void DescriptorPool::send_unsafe_to_manager() {
  this->try_clear_unsafe_pool(false);
  clear_pool(&unsafe_pool_, &this->manager_unsafe_pool());
}


void DescriptorPool::add_to_safe(Descriptor *descr) {
  PoolElement *p = get_elem_from_descriptor(descr);
  p->next(safe_pool_);
  safe_pool_ = p;

  p->descriptor()->on_return_to_pool(this);
  p->cleanup_descriptor();

#ifdef DEBUG_POOL
  p->header().free_count.fetch_add(1);
  assert(p->header().free_count.load() ==
      p->header().allocation_count.load());
#endif
  safe_pool_count_++;
}


void DescriptorPool::add_to_unsafe(Descriptor* descr) {
  PoolElement *p = get_elem_from_descriptor(descr);
  p->next(unsafe_pool_);
  unsafe_pool_ = p;
  unsafe_pool_count_++;

}


void DescriptorPool::try_clear_unsafe_pool(bool dont_check) {
  while (unsafe_pool_) {
    PoolElement *temp = unsafe_pool_->next();
    Descriptor *temp_descr = unsafe_pool_->descriptor();

    bool watched = is_watched(temp_descr);
    if (!dont_check && watched) {
      break;
    } else {

      unsafe_pool_count_--;
      this->free_descriptor(temp_descr, true);
      unsafe_pool_ = temp;
    }
  }

  if (unsafe_pool_ != nullptr) {
    PoolElement *prev = unsafe_pool_;
    PoolElement *temp = unsafe_pool_->next();

    while (temp) {
      Descriptor *temp_descr = temp->descriptor();
      PoolElement *temp3 = temp->next();  // TODO(carlos) terrible name

      bool watched = is_watched(temp_descr);
      if (!dont_check && watched) {
        prev = temp;
        temp = temp3;
      } else {
        this->free_descriptor(temp_descr, true);
        prev->next(temp3);
        temp = temp3;
        unsafe_pool_count_--;
      }
    }
  }
}

}  // namespace rc
}  // namespace memory
}  // namespace tervel
