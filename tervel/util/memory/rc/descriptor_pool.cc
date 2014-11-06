#include "tervel/util/memory/rc/descriptor_pool.h"
#include "tervel/util/memory/rc/descriptor_util.h"

namespace tervel {
namespace util {
namespace memory {
namespace rc {

void DescriptorPool::free_descriptor(tervel::util::Descriptor *descr,
      bool dont_check) {
  uintptr_t safty_check = reinterpret_cast<uintptr_t>(descr);
  assert((safty_check & 0x3) == 0x0);
  if (!dont_check && util::memory::rc::is_watched(descr)) {
    this->add_to_unsafe(descr);
  } else {
    this->add_to_safe(descr);
  }
}


void DescriptorPool::reserve(int num_descriptors) {
  for (int i = 0; i < num_descriptors; ++i) {
    PoolElement *elem = new PoolElement();
    elem->next(safe_pool_);
    safe_pool_ = elem;
    safe_pool_count_++;
  }
}


PoolElement * DescriptorPool::get_from_pool(bool allocate_new) {
  // move any objects from the unsafe list to the safe list so that it's more
  // likely the safe list has something to take from.
  this->try_clear_unsafe_pool();

  PoolElement *ret {nullptr};

  // safe pool has something in it. pop the next item from the head of the list.
  if (!NO_REUSE_RC_DESCR && safe_pool_ != nullptr) {
    ret = safe_pool_;
    safe_pool_ = safe_pool_->next();
    ret->next(nullptr);

#ifdef DEBUG_POOL

    assert(ret->header().free_count.load() ==
        ret->header().allocation_count.load());
#endif

    safe_pool_count_--;
    assert(safe_pool_count_ >=0);
  }

  // allocate a new element if needed
  if (ret == nullptr && allocate_new) {
    ret = new PoolElement();
  }

#ifdef DEBUG_POOL
  // update counters to denote that an item was taken from the pool
  ret->header().allocation_count.fetch_add(1);
  ret->header().descriptor_in_use.store(true);
#endif
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


void DescriptorPool::add_to_safe(tervel::util::Descriptor *descr) {
  PoolElement *p = get_elem_from_descriptor(descr);
  p->cleanup_descriptor();

#ifdef DEBUG_POOL
  p->header().free_count.fetch_add(1);
  p->header().descriptor_in_use.store(false);
  assert(p->header().free_count.load() ==
      p->header().allocation_count.load());
#endif

  p->next(safe_pool_);
  safe_pool_ = p;
  safe_pool_count_++;
}


void DescriptorPool::add_to_unsafe(tervel::util::Descriptor* descr) {
  PoolElement *p = get_elem_from_descriptor(descr);
  p->next(unsafe_pool_);
  unsafe_pool_ = p;
  unsafe_pool_count_++;
}


void DescriptorPool::try_clear_unsafe_pool(bool dont_check) {
  if (unsafe_pool_ != nullptr) {
    PoolElement *prev = unsafe_pool_;
    PoolElement *temp = unsafe_pool_->next();

    tervel::util::Descriptor *temp_descr;
    while (temp) {
      temp_descr = temp->descriptor();
      tervel::util::memory::rc::PoolElement *temp_next = temp->next();

      bool watched = is_watched(temp_descr);
      if (!dont_check && watched) {
        prev = temp;
        temp = temp_next;
      } else {
        this->free_descriptor(temp_descr, true);
        prev->next(temp_next);
        temp = temp_next;
        unsafe_pool_count_--;
      }
    }  // while temp

    /**
     * We check the first element last to allow for cleaner looping code.
     */
    temp = unsafe_pool_->next();
    temp_descr = unsafe_pool_->descriptor();

    bool watched = util::memory::rc::is_watched(temp_descr);
    if (dont_check || !watched) {
      unsafe_pool_count_--;
      this->free_descriptor(temp_descr, true);
      unsafe_pool_ = temp;
    }
  }  // if unsafe_pool_
}

}  // namespace rc
}  // namespace memory
}  // namespace util
}  // namespace tervel
