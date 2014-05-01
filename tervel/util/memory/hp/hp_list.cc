#include "tervel/util/memory/hp/hp_list.h"
#include "tervel/util/memory/hp/hp_element.h"


namespace tervel {
namespace util {
namespace memory {
namespace hp {

void ElementList::free_descriptor(Element *descr, bool dont_check) {
  if (!dont_check && is_watched(descr)) {
    this->add_to_unsafe(descr);
  } else {
    delete descr;
  }
}


void ElementList::send_to_manager() {
  this->try_to_free_elements(false);
  this->manager.free_lists_[tervel::tl_thread_info->get_thread_id()]
          .element_list_ = element_list_;
  element_list_ = nullptr;
}



void ElementList::add_to_unsafe(Element* elem) {
  this->try_to_free_elements(false);
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
      PoolElement *temp_next = temp->next();

      bool watched = is_watched(temp);
      if (!dont_check && watched) {
        prev = temp;
        temp = temp_next;
      } else {
        temp->~Element();
        prev->next(temp_next);
        temp = temp_next;
      }
    }

    /**
     * We check the first element last to allow for cleaner looping code.
     */
    temp = element_list_->next();
    bool watched = is_watched(temp);
    if (dont_check || !watched) {
      temp->~Element();
      element_list_ = temp;
    }
  }
}

}  // namespace hp
}  // namespace memory
}  // namespace util
}  // namespace tervel
