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
#include <tervel/util/memory/hp/hp_list.h>
#include <tervel/util/memory/hp/list_manager.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hazard_pointer.h>


namespace tervel {
namespace util {
namespace memory {
namespace hp {

void ElementList::send_to_manager() {
  this->try_to_free_elements(false);
  const uint64_t tid = tervel::tl_thread_info->get_thread_id();
  this->manager_->recieve_element_list(tid, element_list_);
  element_list_ = nullptr;
}



void ElementList::add_to_unsafe(Element* elem) {
  elem->next(element_list_);
  element_list_ = elem;
}

void ElementList::try_to_free_elements(bool dont_check) {
  /**
   * Loop until no more elements can be freed from the element_list_ linked list
   * OR the first element is not safe to be freed
   */
  #ifdef TERVEL_MEM_HP_NO_WATCH
      assert(false);
  #endif

  if (element_list_ != nullptr) {

    Element *prev = element_list_;
    Element *temp = element_list_->next();

    while (temp) {
      Element *temp_next = temp->next();

      bool watched = HazardPointer::is_watched(temp);
      if (!dont_check && watched) {
        prev = temp;
        temp = temp_next;
      } else {
        #ifndef TERVEL_MEM_HP_NO_FREE
          delete temp;
        #endif
        prev->next(temp_next);
        temp = temp_next;
      }
    }

    /**
     * We check the first element last to allow for cleaner looping structure.
     */
    temp = element_list_->next();
    bool watched = HazardPointer::is_watched(element_list_);
    if (dont_check || !watched) {
      #ifndef TERVEL_MEM_HP_NO_FREE
        delete element_list_;
      #endif
      element_list_ = temp;
    }
  }
}

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel
