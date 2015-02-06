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

        if (vec_->internal_array.is_descriptor(cvalue, spot)) {
          continue;
        } else if (cvalue == Vector<T>::c_not_value_) {
          value_.store(c_fail_value_);
          return;
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
  static const T c_fail_value_ {reinterpret_cast<T>(~0x1L)};
};  // class ReadOp
}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  //__TERVEL_CONTAINERS_WF_VECTOR_READ_OP_H