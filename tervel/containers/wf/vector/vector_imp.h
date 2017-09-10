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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_IMP_
#define __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_IMP_

#include <tervel/util/info.h>
#include <tervel/util/descriptor.h>

#include <tervel/containers/wf/vector/vector.hpp>

#include <tervel/containers/wf/vector/read_op.h>
#include <tervel/containers/wf/vector/write_op.h>
#include <tervel/containers/wf/vector/pushback_op.h>
#include <tervel/containers/wf/vector/pushbackwra_op.h>
#include <tervel/containers/wf/vector/popback_op.h>
#include <tervel/containers/wf/vector/popbackwra_op.h>
#include <tervel/containers/wf/vector/insertAt_op.h>
#include <tervel/containers/wf/vector/eraseAt_op.h>

#include <tervel/containers/wf/vector/vector_array.h>

namespace tervel {
namespace containers {
namespace wf {
namespace vector {

template<typename T>
size_t Vector<T>::push_back_only(T value) {
  if (!internal_array.is_valid(value)) {
    assert(false);
    return -1;
  }

  size_t placed_pos = size(1);
  std::atomic<T> *spot = internal_array.get_spot(placed_pos);

  spot->store(value, std::memory_order_relaxed);
  return placed_pos;
}  // push_back_only

 template<typename T>
  size_t Vector<T>::push_back_w_ra(T value) {
    tervel::util::ProgressAssurance::check_for_announcement();

    if (!internal_array.is_valid(value)) {
      assert(false);
      return -1;
    }

    size_t placed_pos = size();


    tervel::util::ProgressAssurance::Limit progAssur;
    while (progAssur.isDelayed() == false) {
      std::atomic<T> *spot = internal_array.get_spot(placed_pos);
      T expected = spot->load();
      if ( (expected ==  Vector<T>::c_not_value_) &&
                    spot->compare_exchange_weak(expected, value) ) {
        size(1);
        return placed_pos;
      } else if (internal_array.is_descriptor(expected, spot)) {
        continue;
      } else {  // its a valid value
          placed_pos++;
      }
    }

    PushWRAOp<T> *op = new PushWRAOp<T>(this, value);
    util::ProgressAssurance::make_announcement(
          reinterpret_cast<tervel::util::OpRecord *>(op));

    size_t result = op->result();
    op->safe_delete();

    return result;
  }  // push_back_w_ra

template<typename T>
bool Vector<T>::pop_back_only(T &value) {
  size_t poped_pos = size(-1);

  if (poped_pos <= 0) {
    size(1);
    return false;
  } else {
    std::atomic<T> *spot = internal_array.get_spot(poped_pos - 1);
    value = spot->load(std::memory_order_relaxed);
    spot->store(Vector<T>::c_not_value_, std::memory_order_relaxed);

    return true;
  }
}  // pop_back_only


  template<typename T>
  bool Vector<T>::pop_back_w_ra(T &value) {
    tervel::util::ProgressAssurance::check_for_announcement();

    size_t poped_pos = size();


    tervel::util::ProgressAssurance::Limit progAssur;
    while (progAssur.isDelayed() == false) {
      if (poped_pos <= 0) {
        return false;
      }

      // TODO(steven) after working, optimize the below load  in the event the
      // value changes
      std::atomic<T> *spot = internal_array.get_spot(poped_pos - 1);
      T current = spot->load();

      if (current == Vector<T>::c_not_value_) {
        poped_pos--;
        continue;
      } else if (internal_array.is_descriptor(current, spot)) {
        continue;
      }else if (spot->compare_exchange_weak(current, Vector<T>::c_not_value_)) {
        size(-1);
        value = current;
        return true;
      } else {
        poped_pos--;
      }
    }

    PopWRAOp<T> *op = new PopWRAOp<T>(this);
    util::ProgressAssurance::make_announcement(
          reinterpret_cast<tervel::util::OpRecord *>(op));

    bool result = op->result(value);
    op->safe_delete();

    return result;
  }  // pop_back_w_ra


template<typename T>
size_t Vector<T>::push_back(T value) {
  tervel::util::ProgressAssurance::check_for_announcement();

  if(!internal_array.is_valid(value)){
    assert(false);
    return -1;
  }

  size_t pos = PushOp<T>::execute(this, value);

  size(1);
  return pos;
}


template<typename T>
bool Vector<T>::pop_back(T &value) {
  tervel::util::ProgressAssurance::check_for_announcement();

  bool res = PopOp<T>::execute(this, value);

  if (res) {
    size(-1);
  }
  return res;
}

template<typename T>
bool Vector<T>::at(size_t idx, T &value) {
  tervel::util::ProgressAssurance::check_for_announcement();

  std::atomic<void *> control_address(nullptr);
  tervel::tl_control_word = &control_address;

  if (idx < capacity()) {
    std::atomic<T> *spot = internal_array.get_spot(idx, false);

    tervel::util::ProgressAssurance::Limit progAssur;
    while (progAssur.isDelayed() == false) {
      T cvalue = spot->load(std::memory_order_relaxed);

      if (cvalue == Vector<T>::c_not_value_) {
        return false;
      }else if (internal_array.is_descriptor(cvalue, spot)) {
        continue;
      } else {
        assert(internal_array.is_valid(cvalue));
        value = cvalue;
        return true;
      }
    }  // while fail threshold has not been reached

    ReadOp<T> *op = new ReadOp<T>(this, idx);


    util::ProgressAssurance::make_announcement(
          reinterpret_cast<tervel::util::OpRecord *>(op));

    bool op_succ = op->result(value);
    op->safe_delete();

    return op_succ;
  }  // if idx < capacity()

  return false;
};

template<typename T>
bool Vector<T>::cas(size_t idx, T &expected, const T val) {
  assert(internal_array.is_valid(expected));
  assert(internal_array.is_valid(val));

  tervel::util::ProgressAssurance::check_for_announcement();

  std::atomic<void *> control_address(nullptr);
  tervel::tl_control_word = &control_address;

  if (idx < capacity()) {
    std::atomic<T> *spot = internal_array.get_spot(idx, false);

    tervel::util::ProgressAssurance::Limit progAssur;
    while (progAssur.isDelayed() == false) {
      T cvalue = spot->load(std::memory_order_relaxed);

      if (cvalue == c_not_value_) {
        return false;
      } else if (internal_array.is_descriptor(cvalue, spot)) {
        continue;
      } else if (cvalue == expected) {
        T temp = expected;
        bool suc = spot->compare_exchange_strong(temp, val);
        if (suc) {
          return suc;
        }
      } else {
        expected = cvalue;
        return false;
      }
    }  // while fail threshold has not been reached

    WriteOp<T> *op = new WriteOp<T>(this, idx, expected, val);

    util::ProgressAssurance::make_announcement(
          reinterpret_cast<tervel::util::OpRecord *>(op));

    bool op_succ = op->result(expected);
    op->safe_delete();

    return op_succ;
  }  // if idx < capacity()

  expected = Vector<T>::c_not_value_;
  return false;
};


template<typename T>
bool Vector<T>::insertAt(size_t idx, T value){
  // Perform bounds checking
  if(!internal_array.is_valid(value)){
    assert(false);
    return false;
  }

  tervel::util::ProgressAssurance::check_for_announcement();

  // Create operation record
  InsertAt<T>* op = new InsertAt<T>(this, idx, value);
  // Set thread local value equal to control word.
  tl_control_word = op->state();
  op->execute();
  bool success = !op->isFailed();
  if (success) {
    // remove remaining discriptors
    op->cleanup();
    // adjust vector size
    size(-1);
  }
  op->safe_delete();
  return success;
};

template<typename T>
bool Vector<T>::eraseAt(size_t idx, T &value){
  tervel::util::ProgressAssurance::check_for_announcement();

  // Create operation record
  EraseAt<T>* op = new EraseAt<T>(this, idx);
  // Set thread local value equal to control word.
  tl_control_word = op->state();
  op->execute();

  bool success = !op->isFailed();
  if (success) {
    // remove remaining discriptors
    op->cleanup();
    // get value removed by this operation
    op->removedValue(value);
    // adjust vector size
    size(-1);
  }
  op->safe_delete();
  return success;
};

}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif // __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_IMP_
