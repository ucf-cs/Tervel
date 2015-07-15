#ifndef __TERVEL_CONTAINERS_LF_LINKED_LIST_QUEUE_LINKED_LIST_IMP
#define __TERVEL_CONTAINERS_LF_LINKED_LIST_QUEUE_LINKED_LIST_IMP
#include <tervel/containers/lf/linked_list_queue/linkedlist.h>

namespace tervel {
namespace containers {
namespace lf {


template<typename T>
bool
LinkedList<T>::Node::set_next(Node * const other) {
  // TODO: Implement
  Node *temp = nullptr;
  return next_.compare_exchange_strong(temp, other);
}

template<typename T>
void
LinkedList<T>::Accessor::deinit() {
  // TODO: Implement
  if (node_ != nullptr) {
    tervel::util::memory::hp::HazardPointer::unwatch
      (
        tervel::util::memory::hp::HazardPointer::SlotID::SHORTUSE
      );
  }
  node_ = nullptr;
};



template<typename T>
bool
LinkedList<T>::Accessor::init(std::atomic<Node *> *address) {
  // TODO: Implement
  Node *val = address->load();
  bool res = true;
  if (val != nullptr) {
    res = tervel::util::memory::hp::HazardPointer::watch(
    tervel::util::memory::hp::HazardPointer::SlotID::SHORTUSE
    , val, reinterpret_cast<std::atomic<void *> *>(address)
    , val);
    if (res) {
      node_ = val;
    }
  }
  return res;
};


template<typename T>
LinkedList<T>::~LinkedList() {
  Node *node = front_.load();
  while (node != nullptr) {
    Node *temp = node->next();
    node->safe_delete();
    node = temp;
    size_.fetch_add(-1);
  }
};

template<typename T>
bool
LinkedList<T>::back(Accessor &access) {
  while(true) {
    if (access.init(&back_)) {
      return access.isValid();
    }
  }
};

template<typename T>
bool
LinkedList<T>::front(Accessor &access) {
  while(true) {
    if (access.init(&front_)) {
      return access.isValid();
    }
  }
};

template<typename T>
bool
LinkedList<T>::pop_front(Accessor &access) {
  while(true) {
    if (access.init(&front_)) {
      if (access.isValid() == false) {
        return false;
      }
      Node *n = access.node();
      Node *next = n->next();

      if (next == nullptr) {
        bool res = n->set_next(Node::EndMark);
        if (res) {
          Node *temp = n;
          front_.compare_exchange_strong(temp, nullptr);
          temp = n;
          back_.compare_exchange_strong(temp, nullptr);
          size_.fetch_add(-1);
          n->safe_delete();
          assert(n->next() != nullptr);
          return true;
        } else {
          next = n->next();
          assert(next != nullptr);
        }
      }


      if (next == Node::EndMark) {
        return false;
      }
      if (front_.compare_exchange_strong(n, next)) {
        size_.fetch_add(-1);
        n->safe_delete();
        assert(n->next() != nullptr);
        return true;
      }

      access.deinit();
    }
  }
};

template<typename T>
bool
LinkedList<T>::push_back(T &value) {
  Accessor access;
  Node *new_node = new Node(value);
  while(true) {
    if (access.init(&back_)) {
      Node *n = access.node();
      if (n == nullptr) {
        // This makes it non-blocking
        // Need to load front_
        // if it has a node and it is EndMarked remove it.
        // if it does not have a node, place this one, and then set the back?
        // if it has a node and its next is null, set the next the set back

        bool res = back_.compare_exchange_strong(n, new_node);
        if (res) {
          assert(n == nullptr);
          front_.compare_exchange_strong(n, new_node);
          size_.fetch_add(1);
          return true;
        }
        access.deinit();
        continue;

      }

      Node *next = n->next();
      if (next == nullptr) {
        bool res = n->set_next(new_node);
        if (res) {
          back_.compare_exchange_strong(n, new_node);
          size_.fetch_add(1);
          return true;
        }
      } else if (next == Node::EndMark) {
        Node *temp = n;
        front_.compare_exchange_strong(temp, nullptr);
        back_.compare_exchange_strong(n, nullptr);
      } else {
        back_.compare_exchange_strong(n, next);
      }
      access.deinit();
    }
  }
};

template<typename T>
bool
LinkedList<T>::empty() {
  return (front_.load() == nullptr);
};


}  // namespace lf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_LF_LINKED_LIST_QUEUE_LINKED_LIST_IMP