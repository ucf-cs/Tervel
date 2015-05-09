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
#ifndef TERVEL_CONTAINERS_WF_STACK_HELPER_H_
#define TERVEL_CONTAINERS_WF_STACK_HELPER_H_

#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hazard_pointer.h>
#include <tervel/containers/wf/stack/stack.h>

namespace tervel {
namespace containers {
namespace wf {

template<typename T>
class Stack<T>::Helper : public tervel::util::memory::hp::Element {
 public:
  Helper(StackOp *op)
   : op_(op) {}
  ~Helper() {}

  bool on_watch(std::atomic<void *> *address, void *expected) {
    typedef tervel::util::memory::hp::HazardPointer::SlotID SlotID;
    const SlotID pos = SlotID::SHORTUSE2;
    bool res = tervel::util::memory::hp::HazardPointer::watch(pos, op_,
        address, expected);

    if (res) {
      finish(reinterpret_cast<std::atomic<Node *> *>(address),
            reinterpret_cast<Node *>(expected));
      tervel::util::memory::hp::HazardPointer::unwatch(pos);
    }
    return false;
  };

  void finish(std::atomic<Node *> *address, Node *expected) {
    Node * e = expected;
    if (op_->associate(this)) {
      address->compare_exchange_strong(e, new_value_);
    } else {
      address->compare_exchange_strong(e, old_value_);
    }
  };

  StackOp * const op_;
  Node * old_value_;
  Node * new_value_;

};  // class Helper


}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_STACK_HELPER_H_
