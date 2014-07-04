#ifndef __TERVEL_CONTAINERS_WF_VECTOR_WRITE_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_WRITE_OP_H

#include "tervel/util/info.h"
#include "tervel/util/descriptor.h"
#include "tervel/util/progress_assurance.h"
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
    , val_(val) {}

  void help_complete() {
    tervel::tl_control_word = reinterpret_cast< std::atomic<void *>*>(&helper_);

    assert(false);
  };

  bool result(const T &expected) {
    assert(false);
    return false;
  };

 private:
  friend class WriteHelper<T>;
  Vector<T> *vec_;
  size_t idx_;
  T expected_;
  T val_;
  std::atomic<WriteHelper<T> *> helper_ {nullptr};
};  // class WriteOp

template<typename T>
class WriteHelper: public tervel::util::Descriptor {
 public:
  explicit WriteHelper(WriteOp<T> *op)
    : op_(op) {}


  void * complete(void *current, std::atomic<void *> *address) {
    assert(false);
    return nullptr;
  }

  void * get_logical_value() {
    assert(false);
    return nullptr;
  }

  bool on_watch(std::atomic<void *>* address, void * expected) {
    assert(false);
    return false;
  }

  void on_unwatch() {
    assert(false);
  }

  private:
    friend class WriteOp<T>;
    WriteOp<T> *op_;
};  // class WriteHelper
}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_WRITE_OP_H
