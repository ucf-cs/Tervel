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
  class Element;
  class Accessor;
  Stack() {};
  ~Stack() {};

  bool push(T v);
  bool pop(T &v);
 private:
  std::atomic<Element *> _stack{nullptr};  // 15 minutes to find I forgot this initialization
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

  bool load(std::atomic<Element *> *address) {
    Element *element = address->load();
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

  Element * ptr() { return _val; };
 private:
  Element * _val;
};


template<typename T>
bool Stack<T>::push(T v) {
  Element *elem = new Element(v);

  while (true) {
    Accessor access;
    while (access.load(&_stack) == false) {};

    Element *cur = access.ptr();
    if (cur != nullptr) {
      elem->next(cur);
    }

    if (_stack.compare_exchange_strong(cur, elem)) {
      return true;
    } else {
      elem->next(nullptr); // This 9 minutes to figure out that I need to have this here.
    }
  }  // while (true)
}  // bool push(T v)

template<typename T>
bool Stack<T>::pop(T& v) {
  while (true) {
    Accessor access;
    while (access.load(&_stack) == false) {};

    Element *cur = access.ptr();
    Element *next = nullptr;
    if (cur != nullptr) {
      next = cur->next();
    }

    if (cur == nullptr) {
      return false;
    } else if (_stack.compare_exchange_strong(cur, next)) {
      v = cur->value();
      cur->safe_delete();
      return true;
    } else {
      continue;
    }
  }  // while (true)
}  // bool pop(T v)


template<typename T>
class Stack<T>::Element : public tervel::util::memory::hp::Element {
 public:
  Element(T &v) : _val(v) {};
  ~Element() {};
  T value() { return _val; };
  void value(T &v) { _val = v; };
  void next(Element *n) { _next = n; };
  Element *next() { return _next; };
 private:
  T _val;
  Element *_next {nullptr};
};




}  // namespace LF
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_LF_STACK_STACK_H_