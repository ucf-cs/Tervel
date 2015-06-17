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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_ERASEAT_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_ERASEAT_OP_H


#include <tervel/containers/wf/vector/shift_op.h>

namespace tervel {
namespace containers {
namespace wf {
namespace vector {

template<typename T>
class EraseAt : public ShiftOp<T> {
 public:
  EraseAt(Vector<T> *vec, size_t idx)
    : ShiftOp<T>(vec, idx)  {};

  void cleanup();
  virtual T getValue(ShiftHelper<T> * helper);
};

template<typename T>
T EraseAt<T>::getValue(ShiftHelper<T> * helper) {
  assert(this->is_done());
  assert(helper != nullptr);

  helper = helper->next();
  if (helper == nullptr) {
    return Vector<T>::c_not_value_;
  } else {
    return helper->value();
  }
}

template<typename T>
void EraseAt<T>::cleanup() {
  ShiftHelper<T> *helper = ShiftOp<T>::helpers_.load();
  assert(this->is_done());
  assert(helper != nullptr);

  T new_value;
  for (size_t i = ShiftOp<T>::idx_; helper != nullptr; i++) {
    T helper_marked = reinterpret_cast<T>(util::memory::rc::mark_first(helper));
    helper = helper->next();
    if (helper == nullptr) {
      new_value = Vector<T>::c_not_value_;
    } else {
      new_value = helper->value();
    }

    std::atomic<T> *spot = ShiftOp<T>::vec_->internal_array.get_spot(i);
    spot->compare_exchange_strong(helper_marked, new_value);
  }  // For loop
}

}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_ERASEAT_OP_H

