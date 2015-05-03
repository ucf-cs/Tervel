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
#include <tervel/util/memory/rc/pool_manager.h>

namespace tervel {
namespace util {
namespace memory {
namespace rc {

PoolManager::~PoolManager() {
  for (size_t i = 0; i < number_pools_; i++) {
    // Free Unsafe Pools first.
    PoolElement *lst = pools_[i].unsafe_pool.exchange(nullptr);
    while (lst != nullptr) {
      PoolElement *next = lst->next();

      tervel::util::Descriptor *temp_descr;
      temp_descr = lst->descriptor();

      assert(!util::memory::rc::is_watched(temp_descr) &&
        " memory is not being unwatched...");

      delete lst;
      lst = next;
    }

    lst = pools_[i].safe_pool.exchange(nullptr);
    while (lst != nullptr) {
      PoolElement *next = lst->next();

      tervel::util::Descriptor *temp_descr;
      temp_descr = lst->descriptor();

      assert(!util::memory::rc::is_watched(temp_descr) &&
        " memory is not being unwatched and it was in the safe list!...");

      delete lst;
      lst = next;
    }

  }
}

DescriptorPool * PoolManager::allocate_pool(const uint64_t pid) {
  DescriptorPool *pool = new DescriptorPool(this, pid);
  return pool;
}


void PoolManager::get_safe_elements(PoolElement **pool, uint64_t *count, uint64_t min_elem) {
  assert(*pool == nullptr);
  assert(*count == 0);

  for (size_t i = 0; i < number_pools_; i++) {
    PoolElement *temp = pools_[i].safe_pool.load();
    if (temp != nullptr) {
      PoolElement *temp = pools_[i].safe_pool.exchange(nullptr);

      if (temp == nullptr) {
        continue;
      }

      PoolElement *tail = temp;
      (*count)++;
      while (tail->next() != nullptr) {
        (*count)++;
        tail = tail->next();
      }
      tail->next(*pool);
      *pool = temp;

      if (*count >= min_elem) {
        break;
      }
    }
  }  // for
}



void PoolManager::add_safe_elements(uint64_t pid, PoolElement *pool, PoolElement *pool_end) {
  assert(pool != nullptr);

  PoolElement * temp = pools_[pid].safe_pool.load();
  if (temp != nullptr) {
    temp = pools_[pid].safe_pool.exchange(nullptr);
    if (temp != nullptr) {
      if (pool_end == nullptr) {
        pool_end = pool;
        while (pool_end->next() != nullptr) {
          pool_end = pool_end->next();
        }
      }
      assert(pool_end != nullptr);
      pool_end->next(temp);
    }
  }
  assert(pools_[pid].safe_pool.load() == nullptr);
  pools_[pid].safe_pool.store(pool);
  pool = nullptr;
}

void PoolManager::add_unsafe_elements(uint64_t pid, PoolElement *pool) {
  assert(pool != nullptr);
  assert(pools_[pid].unsafe_pool.load() == nullptr && " This should be null, this function is only called inside a destructor...pids being reused?");

  pools_[pid].unsafe_pool.store(pool);
}

}  // namespace rc
}  // namespace memory
}  // namespace util
}  // namespace tervel
