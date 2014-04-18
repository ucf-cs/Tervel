#include "thread/rc/descriptor_pool.h"
#include "thread/util.h"

namespace ucf {
namespace thread {
namespace rc {

void DescriptorPool::free_descriptor(Descriptor *) {}


void DescriptorPool::reserve(int) {}


PoolElement * DescriptorPool::get_from_pool(bool allocate_new) {
  // move any objects from the unsafe list to the safe list so that it's more
  // likely the safe list has something to take from.
  this->try_free_unsafe();

  PoolElement *ret {nullptr};

  // safe pool has something in it. pop the next item from the head of the list.
  if (!NO_REUSE_MEM && safe_pool_ != nullptr) {
    ret = safe_pool_;
    safe_pool_ = safe_pool_->header().next_;
    ret->header().next_ = nullptr;

#ifdef DEBUG_POOL
    // update counters to denote that an item was taken from the pool
    assert(ret->header().free_count_.load() ==
        ret->header().allocation_count_.load());
    ret->header().allocation_count_.fetch_add(1);
    safe_pool_count_ -= 1;
#endif
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


void DescriptorPool::send_safe_to_manager() {}


void DescriptorPool::send_unsafe_to_manager() {}


void DescriptorPool::add_to_safe(Descriptor *descr) {
  PoolElement *p = get_elem_from_descriptor(descr);
  p->header().next_ = safe_pool_;
  safe_pool_ = p;

  // TODO(carlos): make sure that this is the only place stuff is added to the
  // safe list from.
  p->descriptor()->advance_return_to_pool(this);
  p->cleanup_descriptor();

#ifdef DEBUG_POOL
  p->header().free_count_.fetch_add(1);
  assert(p->header().free_count_.load() ==
      p->header().allocation_count_.load());
  safe_pool_count_++;
#endif
}


void DescriptorPool::add_to_unsafe(Descriptor* descr) {
  UNUSED(descr);
}


void DescriptorPool::free_safe() {}


void DescriptorPool::try_free_unsafe(bool dont_check) {
  UNUSED(dont_check);
}


// TODO(carlos) a and value ar just the WORST names for parameters.
bool watch(Descriptor *descr, std::atomic<void *> *a, void *value) {
  PoolElement *elem = get_elem_from_descriptor(descr);
  elem->header().ref_count_.fetch_add(1);
  if (a->load() != value) {
    elem->header().ref_count_.fetch_add(-1);
    return false;
  } else {
    bool res = descr->advance_watch(a, value);
    if (res) {
      return true;
    } else {
      elem->header().ref_count_.fetch_add(-1);
      return false;
    }
  }
}


void unwatch(Descriptor *descr) {
  PoolElement *elem = get_elem_from_descriptor(descr);
  elem->header().ref_count_.fetch_add(-1);
  descr->advance_unwatch();
}


bool is_watched(Descriptor *descr) {
  PoolElement * elem = get_elem_from_descriptor(descr);
  if (elem->header().ref_count_.load() == 0) {
    return descr->advance_is_watched();
  } else {
    return true;
  }
}


}  // namespace rc
}  // namespace thread
}  // namespace ucf
