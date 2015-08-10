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


/**
 * TODO(steven):
 *
 *   Annotate code a bit more.
 *
 */
#ifndef TERVEL_CONTAINERS_WF_STACK_Node_H_
#define TERVEL_CONTAINERS_WF_STACK_Node_H_

#include <tervel/util/info.h>
#include <tervel/util/memory/hp/hp_element.h>

#include <tervel/containers/wf/stack/stack.h>

namespace tervel {
namespace containers {
namespace wf {

template<typename T>
class __attribute__((aligned(CACHE_LINE_SIZE)))  Stack<T>::Node : public tervel::util::memory::hp::Element {
 public:
  Node(T &v) : val_(v) {};
  ~Node() {};
  T value() { return val_; };
  void value(T &v) { val_ = v; };
  void next(Node *n) { next_ = n; };
  Node *next() { return next_; };

  void atomic_next(Node *n) {
    reinterpret_cast<std::atomic<Node *> *>(&next_)->store(n);
  }
  void atomic_next(Node * n1, Node *n2) {
    Node * temp = n1;
    reinterpret_cast<std::atomic<Node *> *>(&next_)->compare_exchange_strong(temp, n2);
  }

  bool on_watch(std::atomic<void *> *address, void *expected) {
    assert(false);
    return false;
  }
 private:
  T val_;
  Node *next_ {nullptr};
};

}  // namespace WF
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_STACK_Node_H_