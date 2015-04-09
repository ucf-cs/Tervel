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
#include <tervel/util/memory/hp/list_manager.h>
#include <tervel/util/memory/hp/hazard_pointer.h>

namespace tervel {
namespace util {
namespace memory {
namespace hp {

ListManager:: ~ListManager() {
  for (size_t i = 0; i < number_pools_; i++) {
    Element * element = free_lists_[i].element_list_.load();
    while (element != nullptr) {
      Element *cur = element;
      element = element->next();

      bool watched = tervel::util::memory::hp::HazardPointer::is_watched(cur);
      assert(!watched && "A Hazard Pointer Protected is still a watched when the list manager is being freed");
      #ifndef TERVEL_MEM_HP_NO_FREE
        delete cur;
      #endif
    }  // While elements to be freed
  }  // for pool
  // delete free_lists_; // std::unique_ptr causes this array to be destroyed
};

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel