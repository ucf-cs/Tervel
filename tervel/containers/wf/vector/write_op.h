#ifndef __TERVEL_CONTAINERS_WF_VECTOR_WRITE_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_WRITE_OP_H

#include "tervel/containers/wf/vector.hpp"
#include "tervel/util/memory/rc/descriptor.h"
#include "tervel/util/memory/rc/descriptor_util.h"
#include "tervel/util/progress_assurance.h"

template<typename T>
class WriteHelper;

template<typename T>
class WriteOp: public tervel::util::OpRecord {
 public:
  class WriteOp(Vector<T> *vec, size_t idx, T expected, T val)
    : vec_(vec)
    , idx_(idx)
    , expected_(expected)
    , val_(val) {}

  void help_complete() {

  }

  bool result(T &expected) {

  }

 private:
  friend class WriteHelper;
  Vector<T> *vec_;
  size_t idx_;
  T expected_, T val_;
  std::atomic<WriteHelper<T> *> helper_{nullptr};
};

template<typename T>
class WriteHelper: public tervel::util::Descriptor {
 public:
  class WriteHelper(WriteOp<T> * op)
    : op_(op){}


  void * complete(void *current, std::atomic<void *> *address) {

  }

  void * get_logical_value() {

  }

  bool on_watch(std::atomic<void *>* address, void * expected) {

  }

  void on_unwatch() {

  }

  private:
    WriteOp<T> *op_;
};

#endif  //__TERVEL_CONTAINERS_WF_VECTOR_WRITE_OP_H
