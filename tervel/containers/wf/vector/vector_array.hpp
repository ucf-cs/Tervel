#ifndef __VECTOR_ARRAY_H
#define __VECTOR_ARRAY_H

#include "cas_op.h"
#include "pop_op.h"
#include "push_op.h"
#include "read_op.h"
#include "eraseat_op.h"
#include "insertat_op.h"



template<class T>
class VectorArray {
 public:
  virtual VectorArray(size_t capacity) = 0;
  virtual ~VectorArray() = 0;

  /**
   * This function returns the address of the specified position
   * @param raw_pos the position
   * @param no_add if true then it will not increase the vectors size
   * @return the address of the specified positon
   */
  virtual std::atomic<T> * get_spot(size_t pos) = 0;

  bool is_descriptor(T, std::atomic<T> * spot) {
    if (util::memory::rc::is_descriptor_first(
            reinterpret_cast<void *>(T))) {


      return true;
    } else {
      // TODO(steven): add asserts
      return false;
    }
  }
};  // class Vector Array

#endif  // __VECTOR_ARRAY_H