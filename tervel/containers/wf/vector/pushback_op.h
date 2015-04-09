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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_PUSH_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_PUSH_OP_H


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
class PushOpHelper;

template<typename T>
class PushDescr;


template<typename T>
class PushOp: public tervel::util::OpRecord {
 public:
  PushOp(Vector<T> *vec, T val)
    : vec_(vec)
    , new_val_(val) {}

  ~PushOp() {
    PushOpHelper<T> * temp = helper_.load();
    assert(temp != nullptr);
    util::memory::rc::free_descriptor(temp, true);
  }

  static size_t execute(Vector<T> *vec, T val) {

    size_t placed_pos = vec->size();

    std::atomic<T> *spot = vec->internal_array.get_spot(placed_pos);

    T current = spot->load();

    tervel::util::ProgressAssurance::Limit progAssur;
    while (progAssur.isDelayed()) {
      if (current ==  Vector<T>::c_not_value_) {
        if (placed_pos == 0) {
          if (spot->compare_exchange_strong(current, val)) {

            return 0;
          } else {
            continue;
          }
        }


        PushDescr<T> *helper = tervel::util::memory::rc::get_descriptor<
            PushDescr<T> >(val, vec);
        T help_t = reinterpret_cast<T>(util::memory::rc::mark_first(helper));
        helper->set_prev_spot(vec->internal_array.get_spot(placed_pos-1));

        if (!spot->compare_exchange_strong(current, help_t)) {
          util::memory::rc::free_descriptor(helper, true);
          continue;
        }
        helper->complete(reinterpret_cast<void *>(help_t),
                reinterpret_cast< std::atomic<void *> *>(spot));
        bool op_res = helper->result();
        util::memory::rc::free_descriptor(helper);

        if (op_res) {
          return placed_pos;
        } else {  // it failed
          // It was not placed correctly, so the vector must have shrunk
          placed_pos--;
          spot = vec->internal_array.get_spot(placed_pos);
          current = spot->load();
        }
      } else if (vec->internal_array.is_descriptor(current, spot)) {
          assert((current & 2) == 0);
          continue;
      } else {  // its a valid value
        placed_pos++;
        spot = vec->internal_array.get_spot(placed_pos);
        current = spot->load();
      }
    }  // while not complete

    // Wait-Free code
//    std::cout << "WF mode ("<< val <<")" << std::endl;
    PushOp<T> * pushOp = new PushOp<T>(vec, val);
    util::ProgressAssurance::make_announcement(reinterpret_cast<
          tervel::util::OpRecord *>(pushOp));
    placed_pos = pushOp->result();



    pushOp->safe_delete();

    return placed_pos;
  };

  uint64_t result() {
    PushOpHelper<T> *temp = helper_.load();
    assert(temp != nullptr);
    return temp->idx();
  }

  void help_complete() {
    tervel::tl_control_word = &helper_;

    PushOpHelper<T> *helper = tervel::util::memory::rc::get_descriptor<
            PushOpHelper<T> >(this);
    T help_t = reinterpret_cast<T>(util::memory::rc::mark_first(helper));


    size_t placed_pos = vec_->size();

    std::atomic<T> *spot = vec_->internal_array.get_spot(placed_pos);
    T current = spot->load();

    while (helper_.load() == nullptr) {
      if (current == Vector<T>::c_not_value_) {
        helper->set_idx(placed_pos);
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
          helper = tervel::util::memory::rc::get_descriptor<
                  PushOpHelper<T> >(this);
          help_t = reinterpret_cast<T>(util::memory::rc::mark_first(helper));

          // It was not placed correctly, the vector must have shrunk
          placed_pos--;
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

  bool is_watched() {
    PushOpHelper<T> * temp = helper_.load();

    if (temp == nullptr) {
      assert(false);  // THis state should not be reached
      return false;
    }
    return temp->is_watched();
  }

 private:
  friend class PushOpHelper<T>;
  Vector<T> *vec_;
  T new_val_;
  std::atomic<PushOpHelper<T> *> helper_ {nullptr};
};  // class PushOp

template<typename T>
class PushDescr: public tervel::util::Descriptor {
 public:
  explicit PushDescr(T val, Vector<T> * vec)
    : val_(val)
    , vec_(vec) {}

  void set_prev_spot(std::atomic<T> * prev_spot) {
    prev_spot_ = prev_spot;
  }

  bool in_progress() {
    return success_.load() == 0;
  }

  bool result() {
    assert(success_.load() != 0);
    return success_.load() == 1;
  }

  bool success() {
    uint64_t temp = 0;
    success_.compare_exchange_strong(temp, 1);
    return temp == 0 || temp == 1;
  }

  bool fail() {
    uint64_t temp = 0;
    success_.compare_exchange_strong(temp, 2);
    return temp == 1;
  }

  using util::Descriptor::get_logical_value;
  void * get_logical_value() {
    uint64_t state = success_.load();
    if (state == 1) {
      return reinterpret_cast<void *>(val_);
    } else {
      return reinterpret_cast<void *>(Vector<T>::c_not_value_);
    }
  }

  using util::Descriptor::complete;
  void * complete(void *value, std::atomic<void *> *address) {
    std::atomic<T> * spot = reinterpret_cast<std::atomic<T> *>(address);
    T current_prev = prev_spot_->load();


    tervel::util::ProgressAssurance::Limit progAssur;
    while (in_progress() && progAssur.isDelayed()) {
      if (current_prev == Vector<T>::c_not_value_) {
        fail();
        break;
      } else if (vec_->internal_array.is_descriptor(current_prev, prev_spot_)) {
        continue;
      } else {
        success();
        break;
      }
    }

    bool op_res;
    if (in_progress()) {
      op_res = fail();
    } else {
      op_res = result();
    }

    T new_current = reinterpret_cast<T>(util::memory::rc::mark_first(this));

    if (op_res) {
      assert(success());
      assert(success_.load() == 1);
      if (spot->compare_exchange_strong(new_current, val_)) {
        new_current = val_;
      }
    } else {
      assert(!success());
      assert(success_.load() == 2);
      if (spot->compare_exchange_strong(new_current,
            reinterpret_cast<T>(Vector<T>::c_not_value_))) {
        new_current = reinterpret_cast<T>(Vector<T>::c_not_value_);
      }
    }

    return reinterpret_cast<void *>(new_current);
  }

  void set_control_word() {
    tervel::tl_control_word = &success_;
  }


 private:
  T val_;
  Vector<T> *vec_;
  std::atomic<T> * prev_spot_ {nullptr};
  std::atomic<uint64_t> success_ {0};
};

template<typename T>
class PushOpHelper: public tervel::util::Descriptor {
 public:
  explicit PushOpHelper(PushOp<T> *op)
    : op_(op) {}

  void set_idx(uint64_t i) {
    idx_ = i;
  };

  uint64_t idx() {
    return idx_;
  };

  bool in_progress() {
    return (success_.load() == 0);
  };

  bool result() {
    assert(success_.load() != 0);
    if (success_.load() == 2) {
      return false;
    } else if (op_->helper_.load() == nullptr) {
      return associate();
    } else if (op_->helper_.load() == this) {
      return true;
    } else {
      assert(op_->helper_.load() != nullptr);
      return false;
    }
  };

  bool associate() {
    assert(success_.load() == 1);
    PushOpHelper *temp_null = nullptr;
    bool res = op_->helper_.compare_exchange_strong(temp_null, this);
    if (res || temp_null == this) {
      assert(op_->helper_.load() == this);
      return true;
    } else {
      assert(op_->helper_.load() != this);
      return false;
    }
  };

  bool success() {
    uint64_t temp = 0;
    if ( (success_.compare_exchange_strong(temp, 1) || temp == 1)
          && associate() ) {
      assert(op_->helper_.load() == this);
      assert(success_.load() == 1);
      return true;
    } else {
      assert(success_.load() == 2);
      return false;
    }
  };

  bool fail() {
    uint64_t temp = 0;
    if (success_.compare_exchange_strong(temp, 2) || temp == 2) {
      return false;
    } else {
      assert(success_.load() == 1);
      if (associate()) {
        return true;
      } else {
        return false;
      }
    }
  };

  using util::Descriptor::complete;
  void * complete(void *value, std::atomic<void *> *address) {
    std::atomic<T> * spot = reinterpret_cast<std::atomic<T> *>(address);

    // Check if placed correctly.
    if (idx_ == 0) {
      success();
    } else {
      std::atomic<T> *spot_prev = op_->vec_->internal_array.get_spot(idx_-1);
      T current_prev = spot_prev->load();

      while (in_progress() && op_->helper_.load() == nullptr) {
        if (current_prev == Vector<T>::c_not_value_) {
          fail();
        } else if (op_->vec_->internal_array.is_descriptor(
              current_prev, spot_prev)) {
          continue;
        } else {
          success();
        }
        break;
      }  // while op not done
    }  // else not first position

    bool op_res;
    if (in_progress()) {
      op_res = fail();
    } else {
      op_res = result();
    }

    T new_current = reinterpret_cast<T>(util::memory::rc::mark_first(this));
    assert(reinterpret_cast<T>(value) == new_current);
    if (op_res) {
      if (spot->compare_exchange_strong(new_current, op_->new_val_)) {
        new_current = op_->new_val_;
      }
    } else {
      if (spot->compare_exchange_strong(new_current,
            reinterpret_cast<T>(Vector<T>::c_not_value_))) {
        new_current = reinterpret_cast<T>(Vector<T>::c_not_value_);
      }
    }

    return reinterpret_cast<void *>(new_current);
  }  // complete

  using util::Descriptor::get_logical_value;
  void * get_logical_value() {
    if (in_progress()) {
      return reinterpret_cast<void *>(Vector<T>::c_not_value_);
    } else {
      return reinterpret_cast<void *>(op_->new_val_);
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

    if (success) {
      complete(value, address);
    }
    return false;
  };


 private:
  PushOp<T> *op_;
  uint64_t idx_;
  std::atomic<uint64_t> success_ {0};
};  // class PushOpHelper

}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_PUSH_OP_H
