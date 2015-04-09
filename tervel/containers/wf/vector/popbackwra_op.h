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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_POPWRA_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_POPWRA_OP_H


#include <tervel/util/info.h>
#include <tervel/util/descriptor.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/hp/hazard_pointer.h>
#include <tervel/util/memory/rc/descriptor_util.h>


#include <tervel/containers/wf/vector/vector.hpp>

namespace tervel {
namespace containers {
namespace wf {
namespace vector {

template<typename T>
class PopWRAOpHelper;

template<typename T>
class PopWRAOp: public tervel::util::OpRecord {
 public:
  static constexpr PopWRAOpHelper<T> * is_empty_const {reinterpret_cast<PopWRAOpHelper<T> *>(0x1L)};

  PopWRAOp(Vector<T> *vec)
    : vec_(vec) {}

  ~PopWRAOp() {
    PopWRAOpHelper<T> * temp = helper_.load();
    assert(temp != nullptr);
    if (temp != is_empty_const) {
      util::memory::rc::free_descriptor(temp, true);
    }

  }

  void set_failed() {
    PopWRAOpHelper<T> *temp = helper_.load();
    if (temp == nullptr) {
      helper_.compare_exchange_strong(temp, is_empty_const);
    }
  }

  void help_complete() {
    size_t placed_pos = vec_->size();
    if (placed_pos <= 0) {
      this->set_failed();
      return;
    }
    std::atomic<T> *spot = vec_->internal_array.get_spot(placed_pos - 1);
    T current_prev = spot->load();

    PopWRAOpHelper<T> *helper = tervel::util::memory::rc::get_descriptor<
            PopWRAOpHelper<T> >(this, current_prev);
    T help_t = reinterpret_cast<T>(util::memory::rc::mark_first(helper));
    tervel::tl_control_word = &helper_;

    while (helper_.load() == nullptr) {
      if (current_prev == Vector<T>::c_not_value_) {
	placed_pos--;
	if (placed_pos <= 0) {
	  this->set_failed();
	  break;
	} else{
	  spot = vec_->internal_array.get_spot(placed_pos - 1);
	  current_prev = spot->load();
	  helper->val_ = current_prev;
	  continue;
	}
      } else if (vec_->internal_array.is_descriptor(current_prev, spot)) {
          continue;
      } else {  // its a valid value

	if (!spot->compare_exchange_strong(current_prev, help_t)) {
	  continue;
	}
	helper->complete(reinterpret_cast<void *>(help_t),
	      reinterpret_cast< std::atomic<void *> *>(spot));
	assert(helper_.load() == helper);
	return;
      }
    }  // while not complete
    util::memory::rc::free_descriptor(helper, true);
  };

  bool result(T &val) {
    PopWRAOpHelper<T> *temp = helper_.load();
    assert(temp != nullptr);
    if (temp == is_empty_const) {
      return false;
    } else {
      return temp->result(val);
    }
  }

  bool result() {
    PopWRAOpHelper<T> *temp = helper_.load();
    assert(temp != nullptr);
    if (temp == is_empty_const) {
      return false;
    } else {
      return temp->result();
    }
  }


 private:
  friend class PopWRAOpHelper<T>;
  Vector<T> *vec_;
  std::atomic<T> value_ {Vector<T>::c_not_value_};
  std::atomic<T> * prev_spot_ {nullptr};
  std::atomic<PopWRAOpHelper<T> *> helper_ {nullptr};
};  // class PopOp

template<typename T>
class PopWRAOpHelper: public tervel::util::Descriptor {
 public:

  PopWRAOpHelper( PopWRAOp<T> *op, T val)
    : val_(val)
    , op_(op) {}

  ~PopWRAOpHelper() {
  }

  using util::Descriptor::complete;
  void * complete(void *value, std::atomic<void *> *address) {

    assert(value == util::memory::rc::mark_first(this));

    bool is_valid = associate();

    void * new_val = reinterpret_cast<void *>(Vector<T>::c_not_value_);
//    if (is_valid) {
//    } else {
//      new_val = reinterpret_cast<void *>(val_);
//    }
    if (address->compare_exchange_strong(value, new_val)) {
      return value;
    } else {
      return new_val;
    }
  }  // complete

  bool associate() {

    PopWRAOpHelper *temp_null = nullptr;
    bool res = op_->helper_.compare_exchange_strong(temp_null, this);
    if (res || temp_null == this) {
      assert(op_->helper_.load() == this);
      return true;
    } else {
      assert(false);
      return false;
    }
  };

  bool result(T &val) {
    if (op_->helper_.load() == nullptr) {
      bool res = associate();
      if(res)
	val = val_;
      return res;
    } else if (op_->helper_.load() == this) {
      val = val_;
      return true;
    } else {
      assert(op_->helper_.load() != nullptr);
      return false;
    }
  }

  bool result() {

    if (op_->helper_.load() == nullptr) {
      return associate();
    } else if (op_->helper_.load() == this) {
      return true;
    } else {
      assert(op_->helper_.load() != nullptr);
      return false;
    }
  }


 private:
  T val_;
  PopWRAOp<T> *op_ {nullptr};
};


}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_POPWRA_OP_H
