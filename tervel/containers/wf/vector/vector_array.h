#ifndef __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_ARRAY_H
#define __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_ARRAY_H

#include "tervel/util/info.h"
#include "tervel/util/descriptor.h"
#include "tervel/util/memory/rc/descriptor_util.h"

namespace tervel {
namespace containers {
namespace wf {
namespace vector {

template<typename T>
class VectorArray {
 public:
  explicit VectorArray() {}
  explicit VectorArray(size_t capacity) {}
  virtual ~VectorArray() {}

  /**
   * This function returns the address of the specified position
   * @param raw_pos the position
   * @param no_add if true then it will not increase the vectors size
   * @return the address of the specified position
   */
  virtual std::atomic<T> * get_spot(const size_t raw_pos,
      const bool no_add = false) = 0;

  virtual bool is_valid(T value) {
    uint64_t val = uint64_t(value);
    val = val & uint64_t(~0x1);
    return val != uint64_t(1);
  }

  /**
   * Overideen by SingleArray model to detect resize.
   * @param  expected [description]
   * @param  spot     [description]
   * @return          [description]
   */
  virtual bool is_descriptor(const T &expected, std::atomic<T> *spot) {
    void *temp = reinterpret_cast<void *>(expected);
    if (util::memory::rc::is_descriptor_first(temp)) {
       /* It is some other threads operation, so lets complete it.*/

      std::atomic<void *> *temp2 =
          reinterpret_cast<std::atomic<void *> *>(spot);
      util::memory::rc::remove_descriptor(temp, temp2);
      return true;
    }
    return false;
  }
};  // class Vector Array
}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_ARRAY_H
