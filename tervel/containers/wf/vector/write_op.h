#ifndef __TERVEL_CONTAINERS_WF_VECTOR_WRITE_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_WRITE_OP_H


#include "tervel/util/info.h"
#include "tervel/util/descriptor.h"
#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/hp/hazard_pointer.h"
#include "tervel/util/memory/rc/descriptor_util.h"


#include "tervel/containers/wf/vector/vector.hpp"

namespace tervel {
namespace containers {
namespace wf {
namespace vector {

template<typename T>
class WriteHelper;


template<typename T>
class WriteOp: public tervel::util::OpRecord {
 public:
  WriteOp(Vector<T> *vec, size_t idx, T expected, T val)
    : vec_(vec)
    , idx_(idx)
    , expected_(expected)
    , new_val_(val) {}

  ~WriteOp() {
    WriteHelper<T> * c_fail_pointer_ =
        reinterpret_cast<WriteHelper<T> *>(~0x1L);
    WriteHelper<T> * temp = helper_.load();
    assert(temp != nullptr);
    if (temp != c_fail_pointer_) {
      util::memory::rc::free_descriptor(temp);
    }
  }

  void help_complete() {
    tervel::tl_control_word = reinterpret_cast< std::atomic<void *>*>(&helper_);
    WriteHelper<T> * c_fail_pointer_ =
        reinterpret_cast<WriteHelper<T> *>(~0x1L);

    if (idx_ < vec_->capacity()) {
      std::atomic<T> *spot = vec_->internal_array.get_spot(idx_, false);

      while (helper_.load() == nullptr) {
        T cvalue = spot->load();

        if (vec_->internal_array.is_descriptor(cvalue, spot)) {
          continue;
        } else if (cvalue == Vector<T>::c_not_value_) {
          break;  // will set to failed
        } else {
          assert(vec_->internal_array.is_valid(cvalue));
          WriteHelper<T> *helper = tervel::util::memory::rc::get_descriptor<
            WriteHelper<T> >(this, cvalue);

          T help_t = reinterpret_cast<T>(util::memory::rc::mark_first(helper));
          bool res = spot->compare_exchange_strong(cvalue, help_t);
          if (res) {
            WriteHelper<T> * helper_null = nullptr;
            res = helper_.compare_exchange_strong(helper_null, helper);
            res = (res || (helper_null == helper));
            assert(helper_.load() !=  nullptr);

            T new_val;
            if (res && (expected_ == cvalue)) {
              new_val = new_val_;
            } else {
              new_val = cvalue;
            }
            spot->compare_exchange_strong(help_t, new_val);

            if (!res) {
              util::memory::rc::free_descriptor(helper);
            }

          } else {
            util::memory::rc::free_descriptor(helper, true);
          }

          return;
        }
      }  // while value_ is c_not_value
    }


    WriteHelper<T> *helper_null = nullptr;
    helper_.compare_exchange_strong(helper_null, c_fail_pointer_);
    assert(helper_.load() !=  nullptr);
  };

  bool result(T &expected) {
    WriteHelper<T> * c_fail_pointer_ =
        reinterpret_cast<WriteHelper<T> *>(~0x1L);
    WriteHelper<T> * temp = helper_.load();
    if (temp == c_fail_pointer_) {
      return false;  // out of bounds exception
    } else if (temp->value() == expected_) {
      return true;
    } else {
      expected = temp->value();
      return false;
    }
    assert(false);
    return false;
  };

  bool is_watched() {
    WriteHelper<T> * temp = helper_.load();

    if (temp == nullptr) {
      assert(false);  // THis state should not be reached
      return false;
    } else if (temp == reinterpret_cast<WriteHelper<T> *>(~0x1L)) {
      return false;
    } else {
      return (helper_.load())->is_watched();
    }
  }

 private:
  friend class WriteHelper<T>;
  Vector<T> *vec_;
  size_t idx_;
  T expected_;
  T new_val_;
  std::atomic<WriteHelper<T> *> helper_ {nullptr};
};  // class WriteOp

template<typename T>
class WriteHelper: public tervel::util::Descriptor {
 public:
  explicit WriteHelper(WriteOp<T> *op, T val)
    : op_(op)
    , val_(val) {}

  T value() {
    return val_;
  }

  using util::Descriptor::complete;
  void * complete(void *value, std::atomic<void *> *address) {
    void * marked = reinterpret_cast<void *>(
          util::memory::rc::mark_first(this));
    assert(marked == value);

    WriteHelper<T> * helper_null = nullptr;
    bool res = op_->helper_.compare_exchange_strong(helper_null, this);
    res = (res || (helper_null == this));
    assert(op_->helper_.load() !=  nullptr);

    void *new_val;
    if (res && (op_->expected_ == val_)) {
      new_val = reinterpret_cast<void *>(op_->new_val_);
    } else {
      new_val = reinterpret_cast<void *>(val_);
    }

    res = address->compare_exchange_strong(marked, new_val);
    if (res) {
      return new_val;
    } else {
      return marked;
    }
  }

  void * get_logical_value() {
    WriteHelper<T> * helper = op_->helper_.load();
    if (helper == this) {
      return reinterpret_cast<void *>(op_->new_val_);
    } else {
      return reinterpret_cast<void *>(val_);
    }
  }

  /**
   * This function is called after this objects rc count was incremented.
   * It acquires a temporary HP watch on the Writeop op, ensures that it is
   * associated, and if so returns true.
   *
   * If it is not associated or it was removed, it returns false
   *
   * @param address the address this WriteHelper was read from
   * @param value the bitmarked value of this WriteHelper
   * @return returns whether or not the watch was successful.
   */
  using util::Descriptor::on_watch;
  bool on_watch(std::atomic<void *> *address, void * value) {
    typedef util::memory::hp::HazardPointer::SlotID t_SlotID;
    bool success = util::memory::hp::HazardPointer::watch(
          t_SlotID::SHORTUSE, op_, address, value);

    if (success) {
      /* Success, means that the WriteOP referenced by this Helper can not
       * be freed while we check to make sure this Helper is associated with
       * it. */
      WriteHelper<T> *helper = op_->helper_.load();
      if (helper == nullptr) {
         if (op_->helper_.compare_exchange_strong(helper, this)) {
           /* If this passed then helper == nullptr, so we set it to be ==
            * this */
            helper = this;
         }
      }

      assert(op_->helper_.load() == helper);

      if (helper != this) {
        /* This Helper was placed in error, remove it and replace it with the
         * logic value of this object (expected_value)
         */
        address->compare_exchange_strong(value, reinterpret_cast<void *>(val_));
        success = false;
      }
      /* No longer need HP protection, if we have RC protection on an associated
       * Helper. If we don't it, the value at this address must have changed and
       * we don't need it either way.
       */
      util::memory::hp::HazardPointer::unwatch(t_SlotID::SHORTUSE);
    }  // End Successfull watch

    if (success) {
      assert(op_->helper_.load() != nullptr);
      assert(util::memory::rc::is_watched(this));
      assert(util::memory::hp::HazardPointer::is_watched(op_));
    }

    return success;
  };

  private:
    friend class WriteOp<T>;
    WriteOp<T> *op_;
    const T val_;
};  // class WriteHelper
}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_WRITE_OP_H
