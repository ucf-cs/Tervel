#ifndef __TERVEL_CONTAINERS_WF_VECTOR_READ_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_READ_OP_H

#include "tervel/containers/wf/vector.hpp"
#include "tervel/util/memory/rc/descriptor.h"
#include "tervel/util/memory/rc/descriptor_util.h"
#include "tervel/util/progress_assurance.h"


template<typename T>
class ReadOp: public tervel::util::OpRecord {
  const static uint64_t FAIL = ~0L;
 public:
  class ReadOp(Vector<T> *vec, size_t idx)
    : vec_(vec)
    , idx_(idx)
    , expected_(expected)
    , val_(val) {}

  void help_complete() {

  }

  bool result(T &expected) {

  }

 private:
  Vector<T> *vec_;
  size_t idx_;
  T expected_, T val_;
  std::atomic<T> value_{nullptr};
};


#endif  //__TERVEL_CONTAINERS_WF_VECTOR_READ_OP_H