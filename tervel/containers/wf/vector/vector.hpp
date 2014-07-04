#ifndef __WFVECTOR_HPP__
#define  __WFVECTOR_HPP__

#include <atomic>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <memory>

#include "tervel/containers/wf/vector_array.h"
#include "tervel/containers/wf/array_array.h"


template<class T>
class Vector{
typedef std::atomic<T> ArrayElement;
 public:
  Vector(const size_t capacity = 64)
    : current_size_(0)
    , internal_array(capacity, c_not_value_) {


  }

  bool at(size_t idx, T &value);
  bool cas(size_t idx, T &expValue, T newValue);

  long push_back_only(T value);
  long push_back_w_ra(T value);
  long push_back(T value);

  bool pop_back_only(T &value);
  bool pop_back_w_ra(T &value);
  bool pop_back(T &value);

  bool insertAt(size_t pos, T value);
  bool eraseAt(size_t pos, T &value);

  size_t size() {
    return current_size_.load();
  };

  size_t capacity() {
    return internal_array.capacity();
  };

 private:
  size_t size(size_t val) {
    return current_size_.fetch_add(val);
  }

  std::atomic<size_t> current_size_{0};
  VectorArray<T> internal_array;
  static const c_not_value_(nullptr);




}; // class Vector

#include "vector.imp"


#endif  // __WFVECTOR_HPP__