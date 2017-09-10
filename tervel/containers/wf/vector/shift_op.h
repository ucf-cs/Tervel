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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_SHIFT_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_SHIFT_OP_H


#include <tervel/util/info.h>
#include <tervel/util/descriptor.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/hp/hazard_pointer.h>
#include <tervel/util/memory/rc/descriptor_util.h>
#include <tervel/containers/wf/vector/vector.hpp>
#include <tervel/containers/wf/vector/shift_helper.h>
#include <tervel/containers/wf/vector/array_array_imp.h>

namespace tervel {
namespace containers {
namespace wf {
namespace vector {

template<typename T>
class InsertAt;
template<typename T>
class EraseAt;

template<typename T>
class ShiftOp: public tervel::util::OpRecord {
 public:
  friend class InsertAt<T>;
  friend class EraseAt<T>;
  friend class ShiftHelper<T>;


  ShiftOp(Vector<T> *vec, size_t idx)
    : idx_(idx)
    , vec_(vec) {};

  ~ShiftOp();

  using tervel::util::OpRecord::on_is_watched;
  bool on_is_watched() {
    ShiftHelper<T> *temp = helpers_.load();
    assert(temp != nullptr);
    if (temp == k_fail_const) {
      return false;
    }
    while (temp != nullptr) {
      if (tervel::util::memory::rc::is_watched(temp)) {
        return true;
      } else {
        temp = temp->next();
      }
    }
    return false;
  };

  // help_complete - is the tervel known function for assisting.
  void help_complete() {
    tl_control_word = state();
    execute(true);
  };

  // execute - is the main entry point into the operation and calls the various
  // functions necessary to complete it.
  void execute(bool announced = false) {
    if (idx_ >= vec_->capacity() || idx_ >= vec_->size()) {
      fail();
      if (isFailed()) {
        return;
      };
    }

    place_first_idx(announced);
    if (tervel::util::RecursiveAction::recursive_return()) {
      return;
    }
    if (isFailed()) {
      return;
    } else {
      place_rest(announced);
      return;
    }
  }


  // place_first_idx - handles the placing of the first helper, once placed, the
  // operation is guaranteed to be successful.
  void place_first_idx(bool announced);
  // place_rest - handles placing the rest of the helpers.
  void place_rest(bool announced);

  // id_done - indicates if the operation has determined success of failure.
  bool is_done() { return is_done_.load(); };
  // set_done - called once success or failure has been determined.
  void set_done() { is_done_.store(true); };

  // state - returns a pointer to is_done_, a non-null indicates the operation
  // is done. This is used by progress assurance scheme.
  void * state() { return &is_done_; };
  // fail - sets the operation to failed.  
  void fail();
  // isFailed - returns whether or not the operation failed.
  bool isFailed();

  // getValue - returns the value of the helper based on the state of the
  // operation.
  virtual T getValue(ShiftHelper<T> * helper) = 0;
 private:
  // wait_free_return encapsulates logic to determine if the function should
  // return in case the operation has been completed or if it should giveup
  // helping another threads opeatation.
  bool wait_free_giveup(
    bool announced, tervel::util::ProgressAssurance::Limit &p);


  size_t idx_;
  Vector<T> *vec_;
  std::atomic<ShiftHelper<T> *> helpers_{nullptr};
  std::atomic<bool> is_done_{false};

  static constexpr ShiftHelper<T> * k_fail_const {
    reinterpret_cast<ShiftHelper<T> *>(0x1L)
  };

};  // class ShiftOp

template<typename T>
ShiftOp<T>::~ShiftOp() {
  ShiftHelper<T> *helper = helpers_.load();
  if (isFailed()) {
    return;
  }
  while (helper != nullptr) {
    ShiftHelper<T> *temp = helper;
    helper = helper->next();
    util::memory::rc::unsafefree_descriptor(temp);
  }

};

template<typename T>
void ShiftOp<T>::fail() {
  ShiftHelper<T> *cur = helpers_.load();
  if (cur == nullptr) {
    if (helpers_.compare_exchange_strong(cur, k_fail_const) ||
      cur == k_fail_const) {
      set_done();
    }
  }
}

template<typename T>
bool ShiftOp<T>::isFailed() {
  return helpers_.load() == k_fail_const;
}


template<typename T>
void ShiftOp<T>::place_first_idx(bool announced) {

  ShiftHelper<T> *helper = tervel::util::memory::rc::get_descriptor<
        ShiftHelper<T> >(this);
  T helper_marked = reinterpret_cast<T>(util::memory::rc::mark_first(helper));

  std::atomic<T> *spot = vec_->internal_array.get_spot(idx_);


  tervel::util::ProgressAssurance::Limit progAssur;

  ShiftHelper<T> * cur_helper = helpers_.load();
  while (cur_helper == nullptr) {
    if (this->wait_free_giveup(announced, progAssur)) {
      break;
    }

    T expected = spot->load();
    helper->set_value(expected);
    if (expected == Vector<T>::c_not_value_) {
      this->fail();
      break;
    } else if (vec_->internal_array.is_descriptor(expected, spot)) {
      // The is_descriptor function changes the value at the address
      if (tervel::util::RecursiveAction::recursive_return() &&
          this != tl_control_word) {
        return;
      }
    } else {  // its a valid value
      if (spot->compare_exchange_strong(expected, helper_marked)) {
        if (!helpers_.compare_exchange_strong(cur_helper, helper) &&
            cur_helper != helper) {
          spot->compare_exchange_strong(helper_marked, expected);
          util::memory::rc::free_descriptor(helper, false);
        } else {
          return;
        }
      }
    }
    cur_helper = helpers_.load();
  }
  util::memory::rc::free_descriptor(helper, true);
}

template<typename T>
void ShiftOp<T>::place_rest(bool announced) {

  ShiftHelper<T> *last_helper = helpers_.load();
  assert(last_helper != k_fail_const);

  ShiftHelper<T> *helper = nullptr;
  T helper_marked;
  for (size_t i = idx_ + 1; !is_done() ; i++) {
    assert(last_helper != nullptr);

    if (last_helper->end(Vector<T>::c_not_value_)) {
      set_done();
      break;
    }

    if (helper == nullptr) {
      helper = tervel::util::memory::rc::get_descriptor<
        ShiftHelper<T> >(this);
      helper_marked = reinterpret_cast<T>(util::memory::rc::mark_first(helper));
    }
    helper->set_prev(last_helper);

    std::atomic<T> *spot = vec_->internal_array.get_spot(i);


    tervel::util::ProgressAssurance::Limit progAssur;
    while (last_helper->notAssociated()) {
      if (this->wait_free_giveup(announced, progAssur)) {
        // Free the helper that we did not need.
        util::memory::rc::safefree_descriptor(helper);
        return;
      }

      T expected = spot->load();
      helper->set_value(expected);
      if (expected != Vector<T>::c_not_value_ &&
          vec_->internal_array.shift_is_descriptor(expected, spot, this)) {
        if (tervel::util::RecursiveAction::recursive_return()) {
          util::memory::rc::safefree_descriptor(helper);
          return;
        };
        continue;
      } else {  // its a valid value or null
        if (spot->compare_exchange_strong(expected, helper_marked)) {
          if (!last_helper->associate(helper)) {
            spot->compare_exchange_strong(helper_marked, expected);
            assert(this->is_done() || this->isFailed());
            util::memory::rc::safefree_descriptor(helper);
            return;
          } else {
            helper = nullptr;
            break;
          }
        } else {
          continue; // value changed
        }
      }
    }  // while loop
    last_helper = last_helper->next();
  }  // For loop

  if (helper != nullptr) {
    util::memory::rc::unsafefree_descriptor(helper);
  }
}

template<typename T>
bool ShiftOp<T>::wait_free_giveup(
  bool announced,
  const tervel::util::ProgressAssurance::Limit &p) {
  // Check if we are delayed, ignore delay if this is helping as a result
  // of progress assurance.
  if (!announced && p.isDelayed()) {
    if (this == tl_control_word) {
      // Delayed while attempting complete the thread's own shift opeation.
      // So we need to make an announcement
      util::ProgressAssurance::make_announcement(
        reinterpret_cast<tervel::util::OpRecord *>(this));
    } else {
      // Delayed while helping another thread's shift operation.
      // So we need to return to our own operation.
      // Note: The clear_recurive_return needs to be called.
      tervel::util::RecursiveAction::set_recursive_return();
    }
    return true;
  }
  return false;
}

}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_SHIFT_OP_H

