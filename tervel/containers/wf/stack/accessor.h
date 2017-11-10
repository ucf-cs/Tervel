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
#ifndef TERVEL_CONTAINERS_WF_STACK_ACCESSOR_H_
#define TERVEL_CONTAINERS_WF_STACK_ACCESSOR_H_

#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/hp/hazard_pointer.h>
#include <tervel/containers/wf/stack/node.h>


namespace tervel {
namespace containers {
namespace wf {

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
  * Additionally, this method attempts to help complete any pending operations
  * occuring at the node pointed to by address. 
  *
  * @param address Address of the std::atomic<Node *> to be loaded.
  *
  * @return true if successful, false otherwise.
  */
  bool load(std::atomic<Node *> *address) {
    bool res = true;
    Node *element = address->load();

    // If the node has been marked by setting it's least signifigant bit to 1, 
    // a Helper object must be created to help complete the pending operation.
    // The helper object must also be placed on watch to protect it in memory. 
    if (tervel::util::is_1st_lsb_1<Node>(element)) {
      Helper * h = reinterpret_cast<Helper *>(tervel::util::set_1st_lsb_0<Node>(element));

      res = tervel::util::memory::hp::HazardPointer::watch(
      watch_pos, h, reinterpret_cast<std::atomic<void *> *>(address)
      , element);

      assert(res == false);
      return false;
    }
    if (element != nullptr) {
      void *temp = reinterpret_cast<void *>(element);
      res = tervel::util::memory::hp::HazardPointer::watch(
      watch_pos, temp, reinterpret_cast<std::atomic<void *> *>(address)
      , temp);
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


} // namespace WF
} // namespace containers
} // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_STACK_ACCESSOR_H_