#ifndef __VECTOR_DEBUG_HPP_
#define __VECTOR_DEBUG_HPP_

template<class T>
void Vector::print_vector(){
  long p;
  printf("Vector (Size: %d): [", size());
  for (p = 0; p < size(); p++) {
    ArrayElement *spot = internal_array.get_spot(p);
    void *cvalue = reinterrupt_cast<void *>(spot->load());

    printf("[%ld %p], ",p, cvalue);
  }

  printf(" ..... ");

  for (p = p; p < capacity(); p++) {
    ArrayElement *spot = internal_array.get_spot(p);
    void *cvalue = reinterrupt_cast<void *>(spot->load());

    if (cvalue != c_not_value_) {
      printf("[%ld %p], ", p,cvalue);
    }
  }

  printf("]\n");
}

#endif  // __VECTOR_DEBUG_HPP_