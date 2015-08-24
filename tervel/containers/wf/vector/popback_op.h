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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_POP_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_POP_OP_H


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
class PopOpHelper;

template<typename T>
class PopOpSubHelper;


template<typename T>
class PopOp: public tervel::util::OpRecord {
 public:
  static constexpr PopOpHelper<T> * is_empty_const {reinterpret_cast<PopOpHelper<T> *>(0x1L)};

  PopOp(Vector<T> *vec)
    : vec_(vec) {}

  ~PopOp() {
    PopOpHelper<T> * temp = helper_.load();
    assert(temp != nullptr);
    if (temp != is_empty_const) {
      util::memory::rc::free_descriptor(temp, true);
    }

  }

  static bool execute(Vector<T> *vec, T &val) {
    size_t placed_pos = vec->size();
    if (placed_pos == 0) {
      return false;
    }

    PopOpHelper<T> *helper = tervel::util::memory::rc::get_descriptor<
            PopOpHelper<T> >(vec);
    T help_t = reinterpret_cast<T>(util::memory::rc::mark_first(helper));
    helper->set_control_word();

    std::atomic<T> *spot = vec->internal_array.get_spot(placed_pos);

    T current = spot->load();

    tervel::util::ProgressAssurance::Limit progAssur;
    while (!progAssur.isDelayed()) {
      if (current ==  Vector<T>::c_not_value_) {

        // else placed_pos > 0
        helper->set_prev_spot(vec->internal_array.get_spot(placed_pos-1));
        if (!spot->compare_exchange_strong(current, help_t)) {
          continue;
        }
        helper->complete(reinterpret_cast<void *>(help_t),
              reinterpret_cast< std::atomic<void *> *>(spot));
        bool op_res = helper->result(val);
        util::memory::rc::free_descriptor(helper);

        if (op_res) {
          return true;
        } else {  // it failed
          placed_pos--;
          if (placed_pos == 0) {
            return false;
          }

          helper = tervel::util::memory::rc::get_descriptor<
                  PopOpHelper<T> >(vec);
          help_t = reinterpret_cast<T>(util::memory::rc::mark_first(helper));
          helper->set_control_word();
          // It was not placed correctly, so the vector must have shrunk

          spot = vec->internal_array.get_spot(placed_pos);
          current = spot->load();
        }
      } else if (vec->internal_array.is_descriptor(current, spot)) {
          continue;
      } else {  // its a valid value
        placed_pos++;
        spot = vec->internal_array.get_spot(placed_pos);
        current = spot->load();
      }
    }  // while not complete

    util::memory::rc::free_descriptor(helper, true);

    // std::cout << "Wait-Free Mode" << std::endl;
    // Wait-Free code
    assert(util::memory::hp::HazardPointer::hasWatch(util::memory::hp::HazardPointer::SlotID::SHORTUSE) == false && "Thread did not release all HP watches");
    PopOp<T> * op = new PopOp<T>(vec);
    util::ProgressAssurance::make_announcement(reinterpret_cast<
          tervel::util::OpRecord *>(op));
    bool op_res = op->result(val);
    op->safe_delete();

    return op_res;
  };

  bool result(T &val) {
    PopOpHelper<T> *temp = helper_.load();
    assert(temp != nullptr);
    if (temp == is_empty_const) {
      return false;
    } else {
      return temp->result(val);
    }
  }

  bool result() {
    PopOpHelper<T> *temp = helper_.load();
    assert(temp != nullptr);
    if (temp == is_empty_const) {
      return false;
    } else {
      return temp->result();
    }
  }

  void set_failed() {
    PopOpHelper<T> *temp = helper_.load();
    if (temp == nullptr) {
      helper_.compare_exchange_strong(temp, is_empty_const);
    }
  }

  void help_complete() {
    size_t placed_pos = vec_->size();
    if (placed_pos == 0) {
      this->set_failed();
      return;
    }
    PopOpHelper<T> *helper = tervel::util::memory::rc::get_descriptor<
            PopOpHelper<T> >(vec_, this);
    T help_t = reinterpret_cast<T>(util::memory::rc::mark_first(helper));
    tervel::tl_control_word = &helper_;



    std::atomic<T> *spot = vec_->internal_array.get_spot(placed_pos);
    T current = spot->load();

    while (helper_.load() == nullptr) {
      if (current == Vector<T>::c_not_value_) {

        // else place_pos > 0

        helper->set_prev_spot(vec_->internal_array.get_spot(placed_pos-1));

        if (!spot->compare_exchange_strong(current, help_t)) {
          continue;
        }
        helper->complete(reinterpret_cast<void *>(help_t),
              reinterpret_cast< std::atomic<void *> *>(spot));

        bool op_res = helper->result();
        if (op_res) {
          return;
        } else {
          util::memory::rc::free_descriptor(helper);

          placed_pos--;
          if (placed_pos == 0) {
            this->set_failed();
            return;
          }

          helper = tervel::util::memory::rc::get_descriptor<
                  PopOpHelper<T> >(vec_, this);
          help_t = reinterpret_cast<T>(util::memory::rc::mark_first(helper));

          // It was not placed correctly, the vector must have shrunk

          spot = vec_->internal_array.get_spot(placed_pos);
          current = spot->load();
        }
      } else if (vec_->internal_array.is_descriptor(current, spot)) {
          continue;
      } else {  // its a valid value
        placed_pos++;
        spot = vec_->internal_array.get_spot(placed_pos);
        current = spot->load();
      }
    }  // while not complete

    util::memory::rc::free_descriptor(helper, true);
  };

  bool on_is_watched() {
    PopOpHelper<T> * temp = helper_.load();

    if (temp == nullptr) {
      assert(false);  // THis state should not be reached
      return false;
    } else if (temp == is_empty_const) {
      return false;
    }
    return tervel::util::memory::rc::is_watched(temp);
  }

 private:
  friend class PopOpHelper<T>;
  Vector<T> *vec_;
  std::atomic<T> * prev_spot_ {nullptr};
  std::atomic<PopOpHelper<T> *> helper_ {nullptr};
};  // class PopOp

template<typename T>
class PopOpHelper: public tervel::util::Descriptor {
 public:
  static constexpr PopOpSubHelper<T> * fail_const {reinterpret_cast<PopOpSubHelper<T> *>(0x1L)};


  PopOpHelper(Vector<T> * vec, PopOp<T> *op)
    : vec_(vec)
    , op_(op) {}

  explicit PopOpHelper(Vector<T> * vec)
    : vec_(vec)
    , op_{nullptr} {}

  ~PopOpHelper() {
    PopOpSubHelper<T> * temp = child_.load();
    if (temp == nullptr) {
      return;
    } else if (temp == fail_const) {
      return;
    } else {
      util::memory::rc::free_descriptor(temp, true);
    }
  }


  void set_prev_spot(std::atomic<T> * prev_spot) {
    prev_spot_ = prev_spot;
  }

  bool in_progress() {
    return child_.load() == nullptr;
  }

  bool result(T &val) {
    PopOpSubHelper<T> *temp = child_.load();
    assert(temp != nullptr);
    if (temp == fail_const) {
      return false;
    } else {
      temp->val(val);
      return true;
    }
  }

  bool result() {
    PopOpSubHelper<T> *temp = child_.load();
    assert(temp != nullptr);
    if (temp == fail_const) {
      return false;
    } else {
      return true;
    }
  }

  void fail() {
    PopOpSubHelper<T> *temp = child_.load();
    if (temp == nullptr) {
      child_.compare_exchange_strong(temp, fail_const);
    }
  }


  using util::Descriptor::get_logical_value;
  void * get_logical_value() {
   return reinterpret_cast<void *>(Vector<T>::c_not_value_);
  }

  using util::Descriptor::complete;
  void * complete(void *value, std::atomic<void *> *address) {
    std::atomic<T> * spot = reinterpret_cast<std::atomic<T> *>(address);
    T help_t = reinterpret_cast<T>(util::memory::rc::mark_first(this));

    T current_prev = prev_spot_->load();



    tervel::util::ProgressAssurance::Limit progAssur;
    while (in_progress()) {
      if (progAssur.isDelayed() || current_prev == Vector<T>::c_not_value_) {
        fail();
        break;
      } else if (vec_->internal_array.is_descriptor(current_prev, prev_spot_)) {
        continue;
      } else {
        PopOpSubHelper<T> *child = tervel::util::memory::rc::get_descriptor<
            PopOpSubHelper<T> >(this, current_prev);
        T child_m = reinterpret_cast<T>(util::memory::rc::mark_first(child));

        if (prev_spot_->compare_exchange_strong(current_prev, child_m)) {
          child->complete(reinterpret_cast<void *>(child_m),
            reinterpret_cast<std::atomic<void *> *>(prev_spot_));

          if (child_.load() != child) {
            util::memory::rc::free_descriptor(child, false);
          }
          break;
        } else {
          util::memory::rc::free_descriptor(child, true);
          continue;
        }
      }
    }  // end while

    if (spot->compare_exchange_strong(help_t,
            reinterpret_cast<T>(Vector<T>::c_not_value_))) {
      return reinterpret_cast<void *>(Vector<T>::c_not_value_);
    } else {
      return reinterpret_cast<void *>(help_t);
    }
  }  // complete

  bool associate(PopOpSubHelper<T> * child) {
    PopOpSubHelper<T> * cur_child = child_.load();
    if (cur_child == nullptr) {
      if (child_.compare_exchange_strong(cur_child, child)) {
        cur_child = child;
      }
    }

    if (cur_child != child) {
      return false;
    }

    if (op_ == nullptr) {
      return true;
    }

    PopOpHelper<T> *temp_null = nullptr;
    bool res = op_->helper_.compare_exchange_strong(temp_null, this);
    if (res || temp_null == this) {
      assert(op_->helper_.load() == this);
      return true;
    } else {
      assert(op_->helper_.load() != this);
      return false;
    }
  };

  /**
   * This function is called after this objects rc count was incremented.
   * It acquires a  HP watch on the PopOp op,
   *
   * @param address the address this PopOpHelper was read from
   * @param value the bitmarked value of this WriteHelper
   * @return returns whether or not the watch was successful.
   */
  using util::Descriptor::on_watch;
  bool on_watch(std::atomic<void *> *address, void * value) {
    if (op_ ==  nullptr) {
      return true;
    }
    typedef util::memory::hp::HazardPointer::SlotID t_SlotID;
    bool success = util::memory::hp::HazardPointer::watch(
          t_SlotID::SHORTUSE, op_, address, value);

    return success;
  };

  using util::Descriptor::on_unwatch;
  void on_unwatch() {
    if (op_ !=  nullptr) {
      typedef util::memory::hp::HazardPointer::SlotID t_SlotID;
      util::memory::hp::HazardPointer::unwatch(t_SlotID::SHORTUSE);
    }
  };

  using util::Descriptor::on_is_watched;
  bool on_is_watched() {
    PopOpSubHelper<T> *temp = child_.load();
    assert(temp != nullptr);
    if (temp == fail_const) {
      return false;
    } else {
      return tervel::util::memory::rc::is_watched(temp);
    }
  };

  void set_control_word() {
    tervel::tl_control_word = &child_;
  }


 private:
  Vector<T> *vec_;
  PopOp<T> *op_ {nullptr};
  std::atomic<T> * prev_spot_ {nullptr};
  std::atomic<PopOpSubHelper<T> *> child_ {nullptr};
};

template<typename T>
class PopOpSubHelper: public tervel::util::Descriptor {
 public:
  explicit PopOpSubHelper(PopOpHelper<T> *parent, T val)
    : val_(val)
    , parent_(parent) {}

  void val(T &val) {
    val = val_;
  };

  using util::Descriptor::get_logical_value;
  void * get_logical_value() {
   return reinterpret_cast<void *>(Vector<T>::c_not_value_);
  }

  using util::Descriptor::complete;
  void * complete(void *value, std::atomic<void *> *address) {
    assert(value == util::memory::rc::mark_first(this));

    bool is_valid = parent_->associate(this);

    void * new_val;
    if (is_valid) {
      new_val = reinterpret_cast<void *>(Vector<T>::c_not_value_);
    } else {
      new_val = reinterpret_cast<void *>(val_);
    }

    if (address->compare_exchange_strong(value, new_val)) {
      return value;
    } else {
      return new_val;
    }
  }  // complete

  /**
   * This function is called after this objects rc count was incremented.
   * It acquires a  HP watch on the PopOp op,
   *
   * @param address the address this PopOpHelper was read from
   * @param value the bitmarked value of this WriteHelper
   * @return returns whether or not the watch was successful.
   */
  using util::Descriptor::on_watch;
  bool on_watch(std::atomic<void *> *address, void * value) {
    assert(value == util::memory::rc::mark_first(this));
    if (tervel::util::memory::rc::watch(parent_, address, value)) {
      complete(value, address);
      tervel::util::memory::rc::unwatch(parent_);
    }
    return false;
  };


 private:
  T val_;
  PopOpHelper<T> *parent_;
};


}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_POP_OP_H
