#ifndef __TERVEL_CONTAINERS_LF_LINKED_LIST_QUEUE_QUEUE__IMP_H_
#define __TERVEL_CONTAINERS_LF_LINKED_LIST_QUEUE_QUEUE__IMP_H_

namespace tervel {
namespace containers {
namespace lf {

template<typename T>
bool
Queue<T>::Node::on_watch(std::atomic<void *> *address, void *expected) {
  // TODO: Implement
  // The node object contains a reference counting based mechanism, increase/decrease the access
  // as part of the memory protectionist procedure.

}

// template<typename T>
// void
// Queue<T>::Node::on_unwatch(std::atomic<void *> *address, void *expected) {
// // Do not change. //
// Un comment if necessary
//   return;
// }

template<typename T>
bool
Queue<T>::Node::on_is_watched() {
  return is_accessed();
}

template<typename T>
void
Queue<T>::Accessor::uninit() {
  // TODO: Implement
  // This function should remove memory protection on the node_ and next_ members
};

template<typename T>
void
Queue<T>::Accessor::unaccess_node_only() {
  // TODO: Implement
  // This function should remove memory protection on the node_ member

};

/**
 * @brief This function is used to safely access the value stored at an address.
 *
 * @details Using Tervel's framework, load the value of `address' and attempt to acquire memory protection.
 *
 *
 * @param node [description]
 * @param address the address to load from
 * @tparam T Data type stored in the queue
 * @return whether or not init was success
 */
template<typename T>
bool
Queue<T>::Accessor::init(Node *node, std::atomic<Node *> *address) {
  // TODO: Implement
  assert(node != nullptr);

  // Add logic here: using HazardPointer apply memory protection on `node' loaded from `address'
  // be aware, the node's on_watch function will be called
  // because it uses an access object, then you do not need to maintain hazardpointer protection

  // Add logic here: using HazardPointer apply memory protection on `node->next' and have it only succeed if node is still at `address'
  // make sure to handle when Hp fails and how effects prev mem protection


};

template<typename T>
Queue<T>::Queue() {
  Node * node = new Node();
  head_.store(node);
  tail_.store(node);
}


template<typename T>
Queue<T>::~Queue() {
  // TODO: Implement
  // Dequeue all elements until the queue is empty

}


template<typename T>
bool
Queue<T>::enqueue(T &value) {
  // TODO: Implement

  Node *node = new Node(value);
  while (true) {
    // Implement this while loop based on the text book.
    // Use access.init to provide safety when dereferenceing the a value loaded from tail_ and that values next point.
    // use set_tail to fixup the state, in the event tail_.load()->next is not null
    Accessor access;

  }
}

template<typename T>
bool
Queue<T>::dequeue(Accessor &access) {
  // TODO: Implement
  while (true) {
    // Implement this while loop based on the text book
    // USe access.init to safeguard access to the value loaded from head_ and that values next member.
    // Don't forget logic related to removing the last element
    // Don't forget to unit access on retries!

  }
};


template<typename T>
bool
Queue<T>::empty() {
  // TODO: Implement
  while (true) {
    // Implement this based on the book, careful if head_.load() == null does not mean empty
    Accessor access;
  }
}

}  // namespace lf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_LF_LINKED_LIST_QUEUE_QUEUE__IMP_H_