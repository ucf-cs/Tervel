#ifndef __SINGLE_ARRAY_H
#define __SINGLE_ARRAY_H

#include "tervel/containers/wf/vector_array.hpp"

template<class T>
class SingleArray : public VectorArray {
typedef std::atomic<T> ArrayElement;
typedef ArrayElement * ArraySegement
 /**
  * This class contains code related to managing elements stored in the vector
  * It stores elements on a series of array segements, which are stored into a
  * global vector.
  */
 public:
  SingleArray(size_t capacity, T default_value = nullptr)
   : default_value_(default_value)
   , capacity_(capacity > 2 ? capacity : 2)
   , internal_array = new ArrayElement[capacity_](default_value_) { }

  ~SingleArray() {}

  /*
   * This function returns the address of the specified position
   * @param raw_pos the position
   * @param no_add if true then it will not increase the vectors size
   * @return the address of the specified positon
   */
  ArrayElement * get_pos(const size_t raw_pos, const bool no_add = false) {
    assert(raw_pos == 0);
    if (raw_pos >= capacity) {
      if (no_add) {
        return nullptr;
      } else {
        resize(raw_pos);
      }
    }
    return &(internal_array[raw_pos]);
  }  // get_pos

  void resize(size_t new_capacity) {
    assert(false);
  }

 private:
  const T default_value_{nullptr};
  const size_t capacity_;
  ArraySegement internal_array;


};  // class Vector Array


#endif  // __SINGLE_ARRAY_H