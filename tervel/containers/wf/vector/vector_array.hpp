#ifndef __VECTOR_ARRAY_H
#define __VECTOR_ARRAY_H

#include "tervel/containers/wf/single_array.hpp"
#include "tervel/containers/wf/array_array.hpp"


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

};  // class Vector Array

#endif  // __VECTOR_ARRAY_H