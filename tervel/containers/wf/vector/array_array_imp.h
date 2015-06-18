
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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_ARRAY_IMP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_ARRAY_IMP_H

#include <tervel/containers/wf/vector/shift_helper.h>
#include <tervel/containers/wf/vector/pushback_op.h>
#include <tervel/containers/wf/vector/popback_op.h>
#include <tervel/containers/wf/vector/vector_array.h>

namespace tervel {
namespace containers {
namespace wf {
namespace vector {



template<typename T>
bool VectorArray<T>::shift_is_descriptor(T &expected, std::atomic<T> *spot, void *op ) {
  void *ptr = reinterpret_cast<void *>(expected);
  std::atomic<void *> *address = reinterpret_cast<std::atomic<void *> *>(spot);
  if (util::memory::rc::is_descriptor_first(ptr) == false) {
    return false;
  }


  tervel::util::RecursiveAction recurse;
  if (tervel::util::RecursiveAction::recursive_return()) {
    expected = Vector<T>::c_not_value_;  // result not used
    return true;
  }

  tervel::util::Descriptor *descr = util::memory::rc::unmark_first(ptr);
  if (util::memory::rc::watch(descr, address, ptr)) {
    // Prevent recursive helping on the same operation
    {
      ShiftHelper<T> * helper = dynamic_cast<ShiftHelper<T>*>(descr);
      if (helper != nullptr && helper->op() == op) {
        util::memory::rc::unwatch(descr);
        return true;
      }
    }
    {
      PushOpHelper<T> * helper = dynamic_cast<PushOpHelper<T>*>(descr);
      if (helper != nullptr) {
        helper->success();
      }
    }
    {
      PopOpHelper<T> * helper = dynamic_cast<PopOpHelper<T>*>(descr);
      if (helper != nullptr) {
        helper->fail();
      }
    }

    expected = reinterpret_cast<T>(descr->complete(ptr, address));
    util::memory::rc::unwatch(descr);
  } else {
    expected = spot->load();
  }
  return true;
}



}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_ARRAY_H
