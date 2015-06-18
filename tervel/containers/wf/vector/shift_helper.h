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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_SHIFT_HELPER_H
#define __TERVEL_CONTAINERS_WF_VECTOR_SHIFT_HELPER_H


#include <tervel/util/info.h>
#include <tervel/util/descriptor.h>
#include <tervel/util/memory/rc/descriptor_util.h>
#include <tervel/containers/wf/vector/shift_op.h>

namespace tervel {
namespace containers {
namespace wf {
namespace vector {

template<typename T>
class ShiftOp;

template<typename T>
class ShiftHelper: public tervel::util::Descriptor {
 public:
  friend ShiftOp<T>;
  ShiftHelper(ShiftOp<T> *op)
    : op_(op)
    , prev_(nullptr) {};

  ShiftHelper * prev() { return prev_; };
  T value() { return value_; };
  ShiftOp<T> * op() { return op_;};
  bool end(T val) { return value_ == val; };
  void set_value(T value) { value_ = value; };
  void set_prev(ShiftHelper<T> *prev) {
    assert(prev->isAssociatedWithMe());
    prev_ = prev;
  }
  bool associate(ShiftHelper<T> *next) {
    ShiftHelper<T> *expected = next_.load();
    if (expected == nullptr && next_.compare_exchange_strong(expected, next)) {
      return true;
    }
    return expected == next;
  };

  bool isAssociatedWithMe() {
    if (prev_ == nullptr) {
      return op_->helpers_.load() == this;
    } else {
      return prev_->next() == this;
    }
  }
  bool associate() {
    ShiftHelper<T> *expected = nullptr;
    if (prev_ == nullptr) {
      return op_->helpers_.compare_exchange_strong(expected, this) || op_->helpers_.load() == this;
    } else {
      return prev_->associate(this);
    }

  };

  bool notAssociated() {
    return (next_.load() == nullptr);
  }

  ShiftHelper<T> * next() { return next_.load(); }


  using util::Descriptor::complete;
  void * complete(void *value, std::atomic<void *> *address) {
    op_->execute();
    if (tervel::util::RecursiveAction::recursive_return()) {
      return nullptr;
    }
    T val = op_->getValue(this);
    void * new_val = reinterpret_cast<void *>(val);
    if (address->compare_exchange_strong(value, new_val)) {
      return new_val;
    } else {
      return value;
    }
  }

  using util::Descriptor::get_logical_value;
  void * get_logical_value() {
    if (op_->is_done()) {
      return reinterpret_cast<void *>(op_->getValue(this));
    } else {
      return reinterpret_cast<void *>(value_);
    }
  }  // get_logical_value

  /**
  * This function is called after this objects rc count was incremented.
  * It acquires a  HP watch on the PushOp op,
  *
  * @param address the address this PushOpHelper was read from
  * @param value the bitmarked value of this WriteHelper
  * @return returns whether or not the watch was successful.
  */
  using util::Descriptor::on_watch;
  bool on_watch(std::atomic<void *> *address, void * value) {
    typedef util::memory::hp::HazardPointer::SlotID t_SlotID;
    bool success = util::memory::hp::HazardPointer::watch(
         t_SlotID::SHORTUSE, op_, address, value);

    if (success == false) {
      return false;
    } else if (this->associate() == false) {
      address->compare_exchange_strong(value, reinterpret_cast<void *>(value_));
      util::memory::hp::HazardPointer::unwatch(t_SlotID::SHORTUSE);
      return false;
    }
    util::memory::hp::HazardPointer::unwatch(t_SlotID::SHORTUSE);
    // We only need to keep watch on this helper
    return true;
  };

 private:
  ShiftOp<T> *op_;
  ShiftHelper<T> *prev_;
  std::atomic<ShiftHelper<T> *> next_{nullptr};
  T value_;
};

}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_SHIFT_HELPER_H
