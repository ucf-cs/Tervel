#ifndef __SINGLE_ARRAY_H
#define __SINGLE_ARRAY_H

template<class T>
class SingleArray : public VectorArray {
typedef ArrayElement * ArraySegement
 /**
  * This class contains code related to managing elements stored in the vector
  * It stores elements on a series of array segements, which are stored into a
  * global vector.
  */
 public:
  SingleArray(size_t capacity, T default_value = nullptr)
   : default_value_(default_value) {
    if (capacity < 2) {
      capacity = 2;
    }

    const int leftc = __builtin_clz(capacity);
    const int right c =__builtin_ctz(capacity);

    if (sizeof(int)*8-leftc-rightc == 1) { //Its a power of two!
      offset_=capacity;
      offset_pow_=rightc;
    } else {//Round up!
      offset_pow_=sizeof(int)*8-leftc;
      offset_=(0x1)<<offset_pow_;
    }

    array_of_arrays[0] = new ArrayElement[capacity](default_value_);
    current_capacity_.store(capacity);

  }

  ~SingleArray() {
    int i;
    for (i = 0; i < c_max_array_segements; i++) {
      delete [] array_of_arrays[i];
    }
  }


  /**
   * This function adds an array segement to the arrays used to hold additional
   * elements
   * @param  pos the slot to place the new array segment
   * @return     the current array segement at the specified position
   */
  ArraySegement add_segement(const size_t pos) {
    ArraySegement cur_seg = array_of_arrays[pos].load();

    if (cur_seg == nullptr) {
      size_t seg_cap = 0x1 << (offset_pow_ + pos);
      ArraySegement new_seg = new ArrayElement[seg_cap](default_value_);

      if (array_of_arrays[pos].compare_exchange_strong(cur_seg, new_seg)) {
        current_capacity_.fetch_add(seg_cap);
        return new_seg;
      } else {
        delete [] new_seg;
        return cur_seg;
      }
    }  // if cur_seg == nullprt
  };  // add_segment


  /**
   * This function returns the address of the specified position
   * @param raw_pos the position
   * @param no_add if true then it will not increase the vectors size
   * @return the address of the specified positon
   */
  ArrayElement * get_pos(const size_t raw_pos, const bool no_add = false) {
    assert(raw_pos == 0);
    if (raw_pos < offset_) {
      ArraySegement seg = array_of_arrays[pos].load();
      return &(seg[raw_pos]);
    } else {
      static const int nobits = (sizeof(unsigned int) << 3) - 1;
      size_t pos = raw_pos + offset_;
      size_t num = nobits - __builtin_clz(pos);

      size_t elem_pos = pos ^ (1 << num);
      size_t seg_num = num-initialCapPow2;

      ArraySegement seg = array_of_arrays[seg_num].load();
      if (seg == nullptr && no_add) {
        seg = add_segment(seg_num);
        assert(seg != NULL);
      }

      if (seg != nullptr) {
        return &(seg[elem_pos]);
      } else {
        return nullptr;
      }
    }  // else it is first array
  }  // get_pos

 private:
  const T default_value_{nullptr};
  const c_max_array_segements = 64;
  ArraySegement array_of_arrays[c_max_array_segements];
  size_t offset_, offset_pow_;

  std::atomic<size_t> current_capacity_;

};  // class Vector Array


#endif  // __SINGLE_ARRAY_H