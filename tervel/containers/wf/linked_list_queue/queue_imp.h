#ifndef __TERVEL_CONTAINERS_WF_LINKED_LIST_QUEUE_QUEUE__IMP_H_
#define __TERVEL_CONTAINERS_WF_LINKED_LIST_QUEUE_QUEUE__IMP_H_

#include <tervel/util/progress_assurance.h>


namespace tervel {
namespace containers {
namespace wf {

template<typename T>
bool
Queue<T>::Node::on_watch(std::atomic<void *> *address, void *expected) {
  // TODO: Copy the LF implementation over
}

template<typename T>
void
Queue<T>::Node::on_unwatch(std::atomic<void *> *address, void *expected) {
// Do not change. //
  return;
}

template<typename T>
bool
Queue<T>::Node::on_is_watched() {
  return is_accessed();
}

template<typename T>
void
Queue<T>::Accessor::uninit() {
  // TODO: Copy LF Implementation
};

template<typename T>
void
Queue<T>::Accessor::unaccess_ptr_only() {
  // TODO: Copy LF Implementation
};

template<typename T>
bool
Queue<T>::Accessor::init(Node *node, std::atomic<Node *> *address) {
  // TODO: Copy LF Implementation
  return true;
};

template<typename T>
Queue<T>::Queue() {
  Node * node = new Node();
  head_.store(node);
  tail_.store(node);
}


template<typename T>
Queue<T>::~Queue() {
  // TODO: Copy LF Implementation
}


template<typename T>
bool
Queue<T>::enqueue(T &value) {
  // TODO: Update the lock-free code to be wait-free
  // use constructs found in  <tervel/util/progress_assurance.h> (Limit, check_for_announcement, etc)

  Node *node = new Node(value);
  while (/* code */) {
    // TODO copy lock-free code
  }

  // TODO: Instantiate an EnqueueOp and make an announcement

  size(1);
  return true;
}

template<typename T>
bool
Queue<T>::dequeue(Accessor &access) {
  // TODO: Update the lock-free code to be wait-free
  // use constructs found in  <tervel/util/progress_assurance.h> (Limit, check_for_announcement, etc)

  while (/* code */) {
    // TODO copy lock-free code

  }

// TODO: Instantiate an DequeueOp and make an announcement

  bool res;
  if (res)
    size(-1);
  return res;
};


template<typename T>
bool
Queue<T>::empty() {
  // TODO: Copy lock-free code, don't wory about making it wait-free
  while (true) {
    Accessor access;
    // code
  }
}

}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_LINKED_LIST_QUEUE_QUEUE__IMP_H_