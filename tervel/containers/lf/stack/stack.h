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

/**
  * This defines the Accessor class, this class simplifies 
  * access to the memory management scheme in tervel.
  * The use of hazard pointers will ensure that no "watched" section
  * of memory is freed or re-used while a thread is still operating 
  * on it. 
  *
  * The following methods are provided:
  *   load
  *   value
  * They are called when a thread needs to access sections of shared
  * memory
  */
template<typename T>
class Stack<T>::Accessor {
 public:
  typedef tervel::util::memory::hp::HazardPointer::SlotID SlotID;
  static const SlotID watch_pos = SlotID::SHORTUSE;
  Accessor() {};
  ~Accessor() {
    tervel::util::memory::hp::HazardPointer::unwatch(watch_pos);
  };

/**
  * The load() method takes the address of a Node and places
  * that address on "watch." This guarantees that the segment of
  * memory will not be freed or re-used until the same segment of
  * memory is unwatched.
  *
  * load() should be called every time a thread needs to operate 
  * on a node in the stack.
  *
  * @param address Address of the std::atomic<Node *> to be loaded.
  *
  * @return true if successful, false otherwise.
  */
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

/**
  * The value() method takes a reference to a Node* in which to store the logical
  * value of the currently watched node. 
  *
  * @param v Reference of type T in which to store the logical value of the Node.
  *
  * @return true if successful, false otherwise.
  */
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

  // When reading a node from the top of the stack, we must first apply the memory protection scheme.
  // We create an accessor class, and attempt load() on the head of the stack. If successful,
  // this atomically loads and "watches" the node at the head of the stack, preventing other threads 
  // from freeing or re-using the node until the access variable has exited scope. 
  while (true) {
    Accessor access;
    if (access.load(&_stack) == false) {
      continue;
    };

    // If successful, access will contain a pointer to a node.
    // It is now safe to carry out the remainder of the push operation.
    Node *cur = access.ptr();
    elem->next(cur);

    if (_stack.compare_exchange_strong(cur, elem)) {
      return true;
    }
  }  // while (true)
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

/**
  * This defines the Node class. This class extends the "Element" class, 
  * enabling the use of hazard pointers with Node objects.  
  */
template<typename T>
class __attribute__((aligned(CACHE_LINE_SIZE))) Stack<T>::Node : public tervel::util::memory::hp::Element {
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