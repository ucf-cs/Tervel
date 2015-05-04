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
#ifndef TERVEL_UTIL_MEMORY_RC_POOL_ELEMENT_H_
#define TERVEL_UTIL_MEMORY_RC_POOL_ELEMENT_H_

#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/system.h>
#include <tervel/util/descriptor.h>

namespace tervel {
namespace util {
namespace memory {
namespace rc {

const long DEBUG_EXPECTED_STAMP = 0xDEADBEEF;

/**
 * This class is used to hold the memory management information (Header) and
 * a descriptor object. It is important to sepearte them to prevent the case
 * where a thread attempts to dereference an object while its type id is being
 * changed.
 */
class PoolElement {
 public:
  /**
   * All the member variables of PoolElement are stored in a struct so that the
   * left over memory for cache padding can be easily calculated.
   */
  struct Header {
    PoolElement *next;
    std::atomic<int64_t> ref_count {0};

#ifdef DEBUG_POOL
    std::atomic<bool> descriptor_in_use {false};
    std::atomic<uint64_t> allocation_count {0};
    std::atomic<uint64_t> free_count {0};
    long debug_pool_stamp = DEBUG_EXPECTED_STAMP;
#endif
  };

  explicit PoolElement(PoolElement *next=nullptr) {
    this->header().next = next;
    assert(this->header().ref_count.load() == 0);
  }

  ~PoolElement() {
    assert(false && "PoolElement should never be deleted, return it to Tervel please");
  }

  /**
   * @brief Returns a pointer to the associated descriptor of this element. This
   * pointer may or may not reference a constructed object.
   * @details Returns a pointer to the associated descriptor of this element. This
   * pointer may or may not reference a constructed object.
   *
   * TODO(carlos) add const versions of these accessors
   *
   * @return a pointer a descriptor type
   */
  Descriptor * descriptor() { return reinterpret_cast<Descriptor*>(this); }

  /**
   * @brief A reference to the header which houses all the special info
   * @details A reference to the header which houses all the special info
   * @return A reference to the header which houses all the
   */
  Header & header() { return header_; }

  /**
   * @brief Helper method for getting the next pointer.
   */
  PoolElement * next() { return header().next; }

  /**
   * @brief Helper method for setting the next pointer.
   */
  void next(PoolElement *next) { header().next = next; }

  /**
   * @brief Constructs a descriptor of the given type within this pool element. Caller
   * must be careful that there's not another descriptor already in use in this
   * element, or memory will be stomped and resources might leak.
   *
   * Call cleanup_descriptor() to call the descriptor's destructor when done
   * with it.
   */
  template<typename DescrType, typename... Args>
  void init_descriptor(Args&&... args);

  /**
   * @brief Should be called by the owner of this element when the descriptor in this
   * element is no longer needed, and it is safe to destroy it. Simply calls the
   * destructor on the internal descriptor.
   */
  void cleanup_descriptor();
 private:
  char padding_[CACHE_LINE_SIZE - sizeof(Header)];
  Header header_;

  DISALLOW_COPY_AND_ASSIGN(PoolElement);
};
static_assert(sizeof(PoolElement) == CACHE_LINE_SIZE,
    "Pool elements should be cache-aligned. Padding calculation is probably"
    " wrong.");

/**
 * @brief If the given descriptor was allocated through a DescriptorPool, then it has
 * an associated PoolElement header. This methods returns that PoolElement.
 *
 * Use with caution as Descriptors not allocated from a pool will not have an
 * associated header, and, thus, the returned value will be to some random
 * place in memory.
 *
 * @param descr The Descriptor pointer allocated through a descriptor pool which
 *   thus has an associated PoolElement.
 * @return The associated PoolElement.
 */
inline PoolElement * get_elem_from_descriptor(tervel::util::Descriptor *descr);


// IMPLEMENTATIONS
// ===============
template<typename DescrType, typename... Args>
void PoolElement::init_descriptor(Args&&... args) {
  static_assert(sizeof(DescrType) <= sizeof(padding_),
      "Descriptor is too large to use in a pool element");
#ifdef DEBUG_POOL
  this->header().descriptor_in_use.store(true);
#endif
  new(descriptor()) DescrType(std::forward<Args>(args)...);
}


inline void PoolElement::cleanup_descriptor() {
#ifdef DEBUG_POOL
  assert(this->header().descriptor_in_use.load());
  this->header().descriptor_in_use.store(false);
#endif
  this->descriptor()->~Descriptor();
}


inline PoolElement * get_elem_from_descriptor(Descriptor *descr) {
  PoolElement *elem = reinterpret_cast<PoolElement *>(descr);
#ifdef DEBUG_POOL
  assert(elem->header().debug_pool_stamp == DEBUG_EXPECTED_STAMP &&
      "Tried to get a PoolElement from a descriptor which does not have an "
      "associated one.  This probably means the user is attempting to free the "
      "descriptor through a DescriptorPool but the descriptor wasn't allocated "
      "through a DescriptorPool to begin with.");
#endif
  return reinterpret_cast<PoolElement *>(elem);
}


}  // namespace rc
}  // namespace memory
}  // namespace util
}  // namespace tervel

#endif  // TERVEL_UTIL_MEMORY_RC_POOL_ELEMENT_H_
