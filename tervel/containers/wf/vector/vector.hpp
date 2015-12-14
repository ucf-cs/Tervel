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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_HPP_
#define __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_HPP_

#include <atomic>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <memory>

#include <tervel/util/util.h>
#include <tervel/containers/wf/vector/array_array.h>

namespace tervel {
namespace containers {
namespace wf {
namespace vector {

template<typename T>
class Vector {
 public:
  explicit Vector(const size_t capacity = 64)
    : current_size_(0)
    , internal_array(capacity, c_not_value_) {}

  bool at(size_t idx, T &value);
  bool cas(size_t idx, T &expValue, const T newValue);

  size_t push_back_only(T value);
  size_t push_back_w_ra(T value);
  size_t push_back(T value);

  bool pop_back_only(T &value);
  bool pop_back_w_ra(T &value);
  bool pop_back(T &value);

  bool insertAt(size_t pos, T value);
  bool eraseAt(size_t pos, T &value);

  int64_t size() {
    int64_t temp = current_size_.load();
    if (temp < 0)
      return 0;
    else
      return temp;
  };

  size_t capacity() {
    return internal_array.capacity();
  };

  static constexpr T c_not_value_ {reinterpret_cast<T>(0x1L)};

  int64_t size(int64_t val) {
    int64_t temp = current_size_.fetch_add(val);
    if (temp < 0)
      return 0;
    else
      return temp;
  }

  std::atomic<int64_t> current_size_ {0};
  ArrayArray<T> internal_array;
};  // class Vector
}
}
}
}

#endif  // __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_HPP_

#include <tervel/containers/wf/vector/vector_imp.h>
