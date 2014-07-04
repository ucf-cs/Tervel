#ifndef __TERVEL_CONTAINERS_WF_VECTOR_READ_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_READ_OP_H

#include "tervel/util/info.h"
#include "tervel/util/progress_assurance.h"

#include "tervel/containers/wf/vector/vector.hpp"

namespace tervel {
namespace containers {
namespace wf {
namespace vector {

template<typename T>
class Vector;

template<typename T>
class ReadOp: public tervel::util::OpRecord {
 public:
  ReadOp(Vector<T> *vec, size_t idx)
    : vec_(vec)
    , idx_(idx) {}

  void help_complete() {
    tervel::tl_control_word = reinterpret_cast< std::atomic<void *> *>(&value_);
    assert(false);
  };

  bool result(const T &expected) {
    assert(false);
    return false;
  };


 private:
  Vector<T> *vec_;
  size_t idx_;
  std::atomic<T> value_ {Vector<T>::c_not_value_};
  static const uint64_t FAIL = ~0L;
};  // class ReadOp
}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  //__TERVEL_CONTAINERS_WF_VECTOR_READ_OP_H