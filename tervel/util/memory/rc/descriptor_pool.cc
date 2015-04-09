/*
The MIT License (MIT)

Copyright (c) 2015 University of Central Florida's Computer Software Engineering
Scalable & Secure Systems (CSE - S3) Lab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <tervel/util/memory/rc/descriptor_pool.h>
#include <tervel/util/memory/rc/descriptor_util.h>

namespace tervel {
namespace util {
namespace memory {
namespace rc {

void DescriptorPool::free_descriptor(tervel::util::Descriptor *descr,
      bool dont_check) {
  uintptr_t safty_check = reinterpret_cast<uintptr_t>(descr);
  assert((safty_check & 0x3) == 0x0);

  this->try_clear_unsafe_pool();

  if (!dont_check && util::memory::rc::is_watched(descr)) {
    this->add_to_unsafe(descr);
  } else {
    this->add_to_safe(descr);
  }

  this->offload();
}

bool DescriptorPool::verify_pool_count(PoolElement *pool, uint64_t count) {
  PoolElement * p_temp = pool;
  uint64_t c_temp = 0;
  while (p_temp != nullptr) {
    c_temp++;
    p_temp = p_temp->next();
  }
  return (c_temp == count);
};


void DescriptorPool::reserve(size_t num_descriptors) {

  manager_->get_safe_elements(&safe_pool_, &safe_pool_count_,
    num_descriptors);

  assert(verify_pool_count(safe_pool_, safe_pool_count_));

  while (safe_pool_count_ < num_descriptors) {
    PoolElement *elem = new PoolElement();
    elem->next(safe_pool_);
    safe_pool_ = elem;
    safe_pool_count_++;
  }

  assert(verify_pool_count(safe_pool_, safe_pool_count_));
}

void DescriptorPool::offload() {
  static_assert( TERVEL_MEM_RC_MIN_NODES >= 0 && TERVEL_MEM_RC_MIN_NODES < TERVEL_MEM_RC_MAX_NODES, "Error bad values for TERVEL_MEM_RC_MIN_NODES and TERVEL_MEM_RC_MAX_NODES");

  if (safe_pool_count_ > TERVEL_MEM_RC_MAX_NODES) {
    uint64_t extra_count = 0;

    PoolElement * tail = safe_pool_;
    while (safe_pool_count_ > TERVEL_MEM_RC_MIN_NODES) {
      tail = tail->next();
      extra_count++;
      safe_pool_count_--;
    }

    PoolElement *extras = safe_pool_;
    safe_pool_ = tail->next();
    tail->next(nullptr);

    assert(verify_pool_count(safe_pool_, safe_pool_count_));
    assert(verify_pool_count(extras, extra_count));

    this->manager_->add_safe_elements(pool_id_, extras, tail);
  }
}


PoolElement * DescriptorPool::get_from_pool(bool allocate_new) {
#ifdef TERVEL_MEM_RC_NO_FREE
  return new PoolElement();
#endif

  PoolElement *res {nullptr};

  // First if local pool is empty go to global
  if (safe_pool_ == nullptr) {
    assert(safe_pool_count_ == 0 && "safe pool count has diverged and no longer equals the number of elements");
    reserve();  // uses default min nodes
  }

  // If safe pool has something in it. pop the next item from the head of the list.
  if (safe_pool_ != nullptr) {
    res = safe_pool_;
    safe_pool_ = safe_pool_->next();
    res->next(nullptr);

#ifdef DEBUG_POOL
    assert(res->header().free_count.load() ==
        res->header().allocation_count.load());
#endif

    safe_pool_count_--;
    assert(safe_pool_count_ >=0 );
  } else if (allocate_new) {  // allocate a new element if needed
    assert(safe_pool_count_ == 0);
    res = new PoolElement();
  }

#ifdef DEBUG_POOL
  // update counters to denote that an item was taken from the pool
  res->header().allocation_count.fetch_add(1);
  res->header().descriptor_in_use.store(true);
#endif

  return res;
}

void DescriptorPool::send_safe_to_manager() {
  if (safe_pool_ != nullptr) {
    assert(safe_pool_count_ > 0);
    this->manager_->add_safe_elements(pool_id_, safe_pool_);
    safe_pool_count_ = 0;
    safe_pool_ = nullptr;
  }
}


void DescriptorPool::send_unsafe_to_manager() {
  this->try_clear_unsafe_pool(false);

  if (unsafe_pool_ != nullptr) {
    assert(unsafe_pool_count_ > 0);
    this->manager_->add_unsafe_elements(pool_id_, unsafe_pool_);
    unsafe_pool_count_ = 0;
    unsafe_pool_ = nullptr;
  }
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
      PoolElement *temp_next = temp->next();

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
