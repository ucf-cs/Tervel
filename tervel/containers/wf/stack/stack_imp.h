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
#ifndef TERVEL_CONTAINERS_WF_STACK_STACK_IMP_H_
#define TERVEL_CONTAINERS_WF_STACK_STACK_IMP_H_

#include <tervel/containers/wf/stack/stack.h>
#include <tervel/util/progress_assurance.h>

namespace tervel {
namespace containers {
namespace wf {

/**
  * The Push() method adds an element to the top of the stack, returning
  * true if the operation is sucessful. 
  * 
  * @param v The value of the element to be added to the stack.
  *
  * @return true if successful, false otherwise.
  */
template<typename T>
bool Stack<T>::push(T v) {
  Node *elem = new Node(v);

  // To guarantee wait freedom, we make use of the tervel announcment table.
  // A thread first checks to see if other threads have made an announcement
  // by calling check_for_announcement. If an anouncement is found, that means
  // some thread is having trouble completing its operation. By having other
  // threads help the troubled thread, we can guarantee system wide progress. 
  tervel::util::ProgressAssurance::check_for_announcement();

  // This limit is a measurement of how many times a thread can fail to complete 
  // its operation before it makes an annoucement.
  util::ProgressAssurance::Limit progAssur;

  while (!progAssur.isDelayed()) {
    Accessor access;
    if (access.load(&lst_) == false) {
      continue;
    };

    Node *cur = access.ptr();
    elem->next(cur);

    if (lst_.compare_exchange_strong(cur, elem)) {
      return true;
    }
  } // while (true)

  // If isDelayed() returns true, we add our operation to the announcement table.
  PushOp *op = new PushOp(this, elem);
  tervel::util::ProgressAssurance::make_announcement(op);
  op->safe_delete();
  return true;
}  // bool push(T v)

/**
  * The Pop() method removes an element from the top of the stack, returning
  * true if the operation is sucessful.
  * 
  * @param v Reference to object in which the value at the top of the stack will be stored.
  *
  * @return true if successful, false otherwise.
  */
template<typename T>
bool Stack<T>::pop(T& v) {
  tervel::util::ProgressAssurance::check_for_announcement();
  util::ProgressAssurance::Limit progAssur;

  while (!progAssur.isDelayed()) {
    Accessor access;
    if (access.load(&lst_) == false) {
      continue;
    };

    Node *cur = access.ptr();
    Node *next = nullptr;
    if (cur != nullptr) {
      next = cur->next();
    }

    if (cur == nullptr) {
      return false;
    } else if (lst_.compare_exchange_strong(cur, next)) {
      v = cur->value();
      cur->safe_delete();
      return true;
    }
  } // while (true)

  PopOp *op = new PopOp(this);
  tervel::util::ProgressAssurance::make_announcement(op);
  bool res = op->result(v);
  op->safe_delete();
  return res;
} // bool pop(T v)


}  // namespace WF
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_STACK_STACK_IMP_H_