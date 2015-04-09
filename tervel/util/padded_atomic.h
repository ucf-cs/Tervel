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
#ifndef TERVEL_UTIL_PADDEDATOMIC_H
#define TERVEL_UTIL_PADDEDATOMIC_H

#include <tervel/util/system.h>
#include <atomic>

namespace tervel {
namespace util {

template<class T>
class PaddedAtomic {
 public:
  explicit PaddedAtomic() {}
  explicit PaddedAtomic(T value) : atomic(value) {}

  T load(std::memory_order memory_order = std::memory_order_seq_cst) {
    return atomic.load(memory_order);
  }
  void store(T value, std::memory_order memory_order
        = std::memory_order_seq_cst) {
    atomic.store(value, memory_order);
  }

  T exchange(T value, std::memory_order memory_order
        = std::memory_order_seq_cst) {
    return atomic.exchange(value, memory_order);
  }

  bool compare_exchange_weak(T& expected, T desired,
        std::memory_order success, std::memory_order failure ) {
    return atomic.compare_exchange_weak(expected, desired, success, failure);
  }

  bool compare_exchange_weak(T& expected, T desired,
        std::memory_order order = std::memory_order_seq_cst ) {
    return atomic.compare_exchange_weak(expected, desired, order);
  }

  bool compare_exchange_strong(T& expected, T desired,
       std::memory_order success, std::memory_order failure ) {
    return atomic.compare_exchange_strong(expected, desired, success, failure);
  }

  bool compare_exchange_strong(T& expected, T desired, std::memory_order order =
        std::memory_order_seq_cst ) {
    return atomic.compare_exchange_strong(expected, desired, order);
  }

  T fetch_add(T arg, std::memory_order memory_order =
        std::memory_order_seq_cst ) {
    return atomic.fetch_add(arg, memory_order);
  }

  std::atomic<T> atomic;

 private:
  char padding[CACHE_LINE_SIZE-sizeof(std::atomic<T>)];
};


}  // namespace util
}  // namespace tervel

#endif  // TERVEL_UTIL_PADDEDATOMIC_H
