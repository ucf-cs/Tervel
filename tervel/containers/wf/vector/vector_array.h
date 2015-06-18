/*
The MIT License (MIT)

Copyright (c) 2015 University of Central Florida's Computer Software Engineering
Scalable & Secure Systems (CSE - S3) Lab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_ARRAY_H
#define __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_ARRAY_H

#include <tervel/util/info.h>
#include <tervel/util/descriptor.h>
#include <tervel/util/memory/rc/descriptor_util.h>



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
    val = val & uint64_t(0x3);
    return val == uint64_t(0);
  }

  /**
   * Overridden by SingleArray model to detect resize.
   * @param  expected [description]
   * @param  spot     [description]
   * @return          [description]
   */
  virtual bool is_descriptor(T &expected, std::atomic<T> *spot);

  /**
   * Overridden by SingleArray model to detect resize.
   * @param  expected [description]
   * @param  spot     [description]
   * @return          [description]
   */
  virtual bool shift_is_descriptor(T &expected, std::atomic<T> *spot);
};  // class Vector Array

template<typename T>
bool VectorArray<T>::is_descriptor(T &expected, std::atomic<T> *spot) {
  void *temp = reinterpret_cast<void *>(expected);
  if (util::memory::rc::is_descriptor_first(temp)) {
     /* It is some other threads operation, so lets complete it.*/

    std::atomic<void *> *temp2 =
        reinterpret_cast<std::atomic<void *> *>(spot);
    expected = reinterpret_cast<T>(util::memory::rc::remove_descriptor(temp,
        temp2));
    return true;
  }
  return false;
};


}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel

#include <tervel/containers/wf/vector/array_array_imp.h>

#endif  // __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_ARRAY_H
