#include "tervel/util/memory/hp/hp_list.h"
#include "tervel/util/memory/hp/hp_element.h"


namespace tervel {
namespace util {
namespace memory {
namespace hp {

void HPList::free_descriptor(Element *descr, bool dont_check) {
  if (!dont_check && is_watched(descr)) {
    this->add_to_unsafe(descr);
  } else {
    delete descr;
  }
}


void HPList::send_to_manager() {
  this->try_to_free_Elements(false);
  clear_pool(&element_list_, &this->manager.to_free_list_());
}


namespace {

/**
 * Generic logic for clearing a pool and sending its elements to another,
 * shared pool. Parameters are passed by pointer because this function changes
 * their values.
 */
void clear_pool(Element **local_pool,
    std::atomic<Element *> *manager_pool) {
  if (local_pool != nullptr) {
    Element *p1 = *local_pool;
    Element *p2 = p1->next();
    while (p2 != nullptr) {
      p1 = p2;
      p2 = p1->next();
    }

    // p1->pool_next's value is updated to the current value
    // after a failed cas. (pass by reference fun times)
    p1->next(manager_pool->load());
    while (!manager_pool->
        compare_exchange_strong(p1->next(), *local_pool)) {
    }
  }
  *local_pool = nullptr;
}

}  // namespace

void HPList::add_to_unsafe(Element* elem) {
  elem->next(element_list_);
  element_list_ = elem;
}


void HPList::try_to_free_Elements(bool dont_check) {
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
}  // namespace tervil
