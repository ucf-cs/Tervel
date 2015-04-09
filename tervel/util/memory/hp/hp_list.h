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
#ifndef TERVEL_UTIL_MEMORY_HP_LIST_H_
#define TERVEL_UTIL_MEMORY_HP_LIST_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>

#include <tervel/util/info.h>
#include <tervel/util/system.h>
#include <tervel/util/descriptor.h>


namespace tervel {
namespace util {
namespace memory {
namespace hp {

class Element;
class ListManager;

/**
 * Defines a list of objects which are stored until they are safe to be freed.
 *
 * The list is represented as a linked list of HP Elements
 *
 * Further, this object has a parent who is shared amongst other threads.
 * When it is to be destroyed, it sends its remaining elements to the
 * parent, relinquishing ownership of said elements.
 */
class ElementList {
 public:
  friend Element;
  explicit ElementList(ListManager *manager) : manager_(manager) {}

  ~ElementList() { this->send_to_manager(); }


 private:
  // -------------------------
  // FOR DEALING WITH MANAGERS
  // -------------------------

  /**
   * Sends all elements managed by this list to the parent.
   */
  void send_to_manager();


  // --------------------------------
  // DEALS WITH UNSAFE LIST
  // --------------------------------
  /**
   * This function adds an Element to the unsafe list.
   * @param elem The element to add
   */
  void add_to_unsafe(Element* elem);

  /**
   * Tries to free elements from the unsafe list.
   * @param dont_check If true, it ignores safety checks
   */
  void try_to_free_elements(bool dont_check = false);


  // -------
  // MEMBERS
  // -------

  /**
   * This list's manager.
   */
  ListManager *manager_;

  /**
   * A linked list of list elements.
   * Elements are freed when they are no longer referenced by other threads.
   */
  Element *element_list_ {nullptr};
};

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel

#endif  // TERVEL_UTIL_MEMORY_HP_LIST_H_
