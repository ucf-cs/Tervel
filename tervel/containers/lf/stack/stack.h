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
#ifndef TERVEL_CONTAINERS_LF_STACK_STACK_H_
#define TERVEL_CONTAINERS_LF_STACK_STACK_H_

#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hazard_pointer.h>

namespace tervel {
namespace containers {
namespace lf {

// TOTAL Dev time: 2 hours 12 minutes
template<typename T>
class Stack {
 public:
  class Node;
  class Accessor;
  Stack()
  : _stack{nullptr} {};
  ~Stack() {};

  bool push(T v);
  bool pop(T &v);
 private:
  std::atomic<Node *> _stack __attribute__((aligned(CACHE_LINE_SIZE)));
};  // class Stack


template<typename T>
class Stack<T>::Accessor {
 public:
  typedef tervel::util::memory::hp::HazardPointer::SlotID SlotID;
  static const SlotID watch_pos = SlotID::SHORTUSE;
  Accessor() {};
  ~Accessor() {
    tervel::util::memory::hp::HazardPointer::unwatch(watch_pos);
  };

  bool load(std::atomic<Node *> *address) {
    Node *element = address->load();
    bool res = true;
    if (element != nullptr) {
      res = tervel::util::memory::hp::HazardPointer::watch(
      watch_pos, element, reinterpret_cast<std::atomic<void *> *>(address)
      , element);
    }

    if (res) {
      _val = element;
      return true;
    } else {
      return false;
    }
  };

  bool value(T &v) {
    if (_val == nullptr) {
      return false;
    } else {
      v = _val->value();
      return true;
    }
  };

  Node * ptr() { return _val; };
 private:
  Node * _val;
};


template<typename T>
bool Stack<T>::push(T v) {
  Node *elem = new Node(v);

  while (true) {
    Accessor access;
    if (access.load(&_stack) == false) {
      continue;
    };

    Node *cur = access.ptr();
    elem->next(cur);

    if (_stack.compare_exchange_strong(cur, elem)) {
      return true;
    }
  }  // while (true)
}  // bool push(T v)

template<typename T>
bool Stack<T>::pop(T& v) {
  while (true) {
    Accessor access;
    if (access.load(&_stack) == false) {
      continue;
    };

    Node *cur = access.ptr();
    Node *next = nullptr;
    if (cur != nullptr) {
      next = cur->next();
    }

    if (cur == nullptr) {
      return false;
    } else if (_stack.compare_exchange_strong(cur, next)) {
      v = cur->value();
      cur->safe_delete();
      return true;
    }
  }  // while (true)
}  // bool pop(T v)


template<typename T>
class Stack<T>::Node : public tervel::util::memory::hp::Element {
 public:
  Node(T &v) : _val(v) {};
  ~Node() {};
  T value() { return _val; };
  void value(T &v) { _val = v; };
  void next(Node *n) { _next = n; };
  Node *next() { return _next; };
 private:
  T _val;
  Node *_next {nullptr};
};

}  // namespace LF
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_LF_STACK_STACK_H_