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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_READ_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_READ_OP_H

#include <tervel/util/info.h>
#include <tervel/util/progress_assurance.h>

#include <tervel/containers/wf/vector/vector.hpp>

namespace tervel {
namespace containers {
namespace wf {
namespace vector {


template<typename T>
class ReadOp: public tervel::util::OpRecord {
 public:
  ReadOp(Vector<T> *vec, size_t idx)
    : vec_(vec)
    , idx_(idx) {}

  void help_complete() {
    tervel::tl_control_word = reinterpret_cast< std::atomic<void *> *>(&value_);
    if (idx_ < vec_->capacity()) {
      std::atomic<T> *spot = vec_->internal_array.get_spot(idx_, false);

      while (value_.load() == Vector<T>::c_not_value_) {
        T cvalue = spot->load();


        if (cvalue == Vector<T>::c_not_value_) {
          value_.store(c_fail_value_);
          return;
        } else if (vec_->internal_array.is_descriptor(cvalue, spot)) {
          continue;
        } else {
          assert(vec_->internal_array.is_valid(cvalue));
          value_.store(cvalue);
          return;
        }
      }  // while value_ is c_not_value
    }  // if idx < capacity()

    value_.store(c_fail_value_);
  };

  bool result(T &expected) {
    T temp = value_.load();
    if (temp == c_fail_value_) {
      return false;
    } else {
      expected = temp;
      return true;
    }
  };


 private:
  Vector<T> *vec_;
  size_t idx_;
  std::atomic<T> value_ {Vector<T>::c_not_value_};
  static const T c_fail_value_ {static_cast<T>(~0x1L)};
};  // class ReadOp
}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  //__TERVEL_CONTAINERS_WF_VECTOR_READ_OP_H
