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
#ifndef TERVEL_MEMORY_RC_POOL_MANAGER_H_
#define TERVEL_MEMORY_RC_POOL_MANAGER_H_

#include <atomic>
#include <memory>
#include <utility>

#include <assert.h>
#include <stdint.h>

#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/system.h>
#include <tervel/util/descriptor.h>

namespace tervel {
namespace util {
namespace memory {
namespace rc {

class DescriptorPool;
class PoolElement;

/**
 * Encapsulates a shared central pool between several thread-local pools. Idea
 * is that each thread gets a local pool to grab descriptors from, and that each
 * of these pools is managed by a single instance of this class. The thread
 * local pools can periodically release their unused elements into the shared
 * pool in this manager, or can take elements from the shared pools in this
 * manager.
 */
class PoolManager {
 public:
  friend class DescriptorPool;

  explicit PoolManager(size_t number_pools)
      : number_pools_(number_pools)
      , pools_(new ManagedPool[number_pools]) {}

  ~PoolManager();

  /**
   * Allocates a pool for thread-local use.
   */
  DescriptorPool * allocate_pool(const uint64_t pid);

  void get_safe_elements(PoolElement **pool, uint64_t *count, uint64_t min_elem);
  void add_safe_elements(uint64_t pid, PoolElement *pool, PoolElement *pool_end = nullptr);
  void add_unsafe_elements(uint64_t pid, PoolElement *pool);


  const size_t number_pools_;

 private:
  struct ManagedPool {
    std::atomic<PoolElement *> safe_pool {nullptr};
    std::atomic<PoolElement *> unsafe_pool {nullptr};

    char padding[CACHE_LINE_SIZE  - sizeof(safe_pool) - sizeof(unsafe_pool)];
  };
  static_assert(sizeof(ManagedPool) == CACHE_LINE_SIZE,
      "Managed pools have to be cache aligned to prevent false sharing.");

  std::unique_ptr<ManagedPool[]> pools_;

  DISALLOW_COPY_AND_ASSIGN(PoolManager);
};

}  // namespace rc
}  // namespace memory
}  // namespace util
}  // namespace tervel

#endif  // TERVEL_MEMORY_RC_POOL_MANAGER_H_
