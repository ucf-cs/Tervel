

namespace ucf {
namespace thread {
namespace hp {

void HPList::free_descriptor(HPElement *descr, bool dont_check) {
  if (!dont_check && is_watched(descr)) {
    this->add_to_unsafe(descr);
  } else {
    delete descr;
  }
}


void HPList::send_to_manager() {
  this->try_to_free_HPElements(false);
  clear_pool(&unsafe_list, &this->manager_unsafe_list());
}


namespace {

/**
 * Generic logic for clearing a pool and sending its elements to another,
 * shared pool. Parameters are passed by pointer because this function changes
 * their values.
 */
void clear_pool(HPElement **local_pool,
    std::atomic<HPElement *> *manager_pool) {
  if (local_pool != nullptr) {
    HPElement *p1 = *local_pool;
    HPElement *p2 = p1->next();
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

void HPList::add_to_unsafe(HPElement* elem) {
  elem->next(unsafe_list);
  unsafe_list = elem;
}


void HPList::try_to_free_HPElements(bool dont_check) {

  // Loop until either the unsafe_list is empty
  // OR the first element is not safe to be freed
  while (unsafe_list) {
    HPElement *temp = unsafe_list->next();

    // Checks if HPElement is watched
    // And if it is not watched then its destructor is called.
    bool watched = is_watched(temp);
    if (!dont_check && watched) {
      break;
    } else {
      temp->~HPElement();
      unsafe_list = temp;
    }
  }

  // Loop until no more elements can be freed from the unsafe_list linked list
  // OR the first element is not safe to be freed
  if (unsafe_list != nullptr) {
    HPElement *prev = unsafe_list;
    HPElement *temp = unsafe_list->next();

    while (temp) {
      PoolElement *temp_next = temp->next();

      bool watched = is_watched(temp);
      if (!dont_check && watched) {
        prev = temp;
        temp = temp_next;
      } else {
        temp->~HPElement();
        prev->next(temp_next);
        temp = temp_next;
      }
    }
  }
}

}  // namespace hp
}  // namespace thread
}  // namespace ucf
