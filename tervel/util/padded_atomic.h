#ifndef TERVEL_UTIL_PADDEDATOMIC_H
#define TERVEL_UTIL_PADDEDATOMIC_H

#include "tervel/util/system.h"
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
