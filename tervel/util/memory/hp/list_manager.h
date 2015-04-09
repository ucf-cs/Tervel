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
#ifndef TERVEL_UTIL_MEMORY_HP_LIST_MANAGER_H_
#define TERVEL_UTIL_MEMORY_HP_LIST_MANAGER_H_

#include <atomic>
#include <memory>
#include <utility>

#include <assert.h>
#include <stdint.h>


#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/system.h>

#include <tervel/util/memory/hp/hp_list.h>
#include <tervel/util/memory/hp/hp_element.h>



namespace tervel {
namespace util {
namespace memory {
namespace hp {

class ElementList;
class Element;

/**
 * Encapsulates a shared central 'to free list' between several thread-local
 * lists. When a thread is destroyed it will send its unfreeable items to this
 * list, which is freed by the user.
 */
class ListManager {
 public:
  explicit ListManager(size_t number_pools)
      : free_lists_(new ManagedPool[number_pools])
      , number_pools_(number_pools) {}

  ~ListManager();

  ElementList * allocate_list() {
    return new ElementList(this);
  }

 private:
  struct ManagedPool {
    std::atomic<Element *> element_list_ {nullptr};
  };

  /**
   * This function is called when a thread is detached. It moves elements from its
   * private HP pool to the shared pool.
   *
   * @param tid          The threads tervel id
   * @param element_list The list of elements that it owned.
   */
  void recieve_element_list(uint64_t tid, Element * element_list) {
    assert(free_lists_[tid].element_list_.load() == nullptr && "The HP shared free_lists should be empty when this function is called");
    free_lists_[tid].element_list_.store(element_list);
  };

  std::unique_ptr<ManagedPool[]> free_lists_;
  size_t number_pools_;

  friend class ElementList;
  DISALLOW_COPY_AND_ASSIGN(ListManager);
};

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel
#endif  // TERVEL_UTIL_MEMORY_HP_LIST_MANAGER_H_
