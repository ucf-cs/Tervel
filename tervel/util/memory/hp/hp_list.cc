#include "tervel/util/memory/hp/hp_list.h"
#include "tervel/util/memory/hp/list_manager.h"
#include "tervel/util/memory/hp/hp_element.h"
#include "tervel/util/memory/hp/hazard_pointer.h"


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
        if (NO_DELETE_HP_ELEMENTS) {
          temp->~Element();
        } else {
          delete temp;
        }
        prev->next(temp_next);
        temp = temp_next;
      }
    }

    /**
     * We check the first element last to allow for cleaner looping code.
     */
    temp = element_list_->next();
    bool watched = HazardPointer::is_watched(element_list_);
    if (dont_check || !watched) {
      if (NO_DELETE_HP_ELEMENTS) {
        element_list_->~Element();
      } else {
        delete element_list_; 
      }
      element_list_ = temp;
    }
  }
}

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel
