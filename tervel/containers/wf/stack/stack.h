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
#ifndef TERVEL_CONTAINERS_WF_STACK_STACK_H_
#define TERVEL_CONTAINERS_WF_STACK_STACK_H_

#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hazard_pointer.h>

namespace tervel {
namespace containers {
namespace wf {

// Start: 7:52pm
template<typename T>
class Stack {
 public:


  Stack()
   : lst_(nullptr) {};
  ~Stack() {};

  bool push(T v);
  bool pop(T &v);

  class Node;
  class Accessor;
  class Helper;
  class StackOp;
  class PopOp;
  class PushOp;

  DISALLOW_COPY_AND_ASSIGN(Stack);
 private:
  std::atomic<Node *> lst_;
};  // class Stack


}  // namespace WF
}  // namespace containers
}  // namespace tervel

#include <tervel/containers/wf/stack/helper.h>
#include <tervel/containers/wf/stack/accessor.h>
#include <tervel/containers/wf/stack/node.h>
#include <tervel/containers/wf/stack/stack_op.h>
#include <tervel/containers/wf/stack/stack_imp.h>

#endif  // TERVEL_CONTAINERS_WF_STACK_STACK_H_