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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_INSERTAT_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_INSERTAT_OP_H


#include <tervel/containers/wf/vector/shift_op.h>

namespace tervel {
namespace containers {
namespace wf {
namespace vector {

template<typename T>
class InsertAt : public ShiftOp<T> {
 public:
  InsertAt(Vector<T> *vec, size_t idx, T val)
    : ShiftOp<T>(vec, idx)
    , val_(val) {};

  void cleanup();
  virtual T getValue(ShiftHelper<T> * helper);

 private:
  T val_;
};

template<typename T>
T InsertAt<T>::getValue(ShiftHelper<T> * helper) {
  assert(this->is_done());
  assert(helper == nullptr);
  if (ShiftOp<T>::helpers_.load() == helper) {
    return val_;
  } else {
    return helper->prev()->value();
  }
}

template<typename T>
void InsertAt<T>::cleanup() {
  ShiftHelper<T> *helper = ShiftOp<T>::helpers_.load();
  assert(this->is_done());
  assert(helper != nullptr);

  T helper_marked = reinterpret_cast<T>(util::memory::rc::mark_first(helper));
  std::atomic<T> *spot = ShiftOp<T>::vec_->internal_array.get_spot(ShiftOp<T>::idx_);
  T new_value = this->val_;
  spot->compare_exchange_strong(helper_marked, new_value);


  for (size_t i = ShiftOp<T>::idx_ + 1; ; i++) {
    new_value = helper->value();
    helper = helper->next();
    if (helper == nullptr) {
      return;
    }

    helper_marked = reinterpret_cast<T>(util::memory::rc::mark_first(helper));
    spot = ShiftOp<T>::vec_->internal_array.get_spot(i);
    spot->compare_exchange_strong(helper_marked, new_value);
  }  // For loop
}

}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_INSERTAT_OP_H

