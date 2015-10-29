
#ifndef __TERVEL_CONTAINERS_WF_LINKED_LIST_QUEUE_OP_RECORD_H_
#define __TERVEL_CONTAINERS_WF_LINKED_LIST_QUEUE_OP_RECORD_H_

#include <tervel/util/util.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hazard_pointer.h>

#include <tervel/containers/wf/linked_list_queue/queue.h>

namespace tervel {
namespace containers {
namespace wf {

template<typename T>
class Queue<T>::Op : public util::OpRecord {
 public:
  static const tervel::util::memory::hp::HazardPointer::SlotID kSlot =
    tervel::util::memory::hp::HazardPointer::SlotID::SHORTUSE2;

  class Helper : public tervel::util::memory::hp::Element {
   public:
    Helper(Op * op) : op_(op) {};

    bool on_watch(std::atomic<void *> *address, void *expected) {
      // TODO implement this function to acquire HazardPointer protection on op and then remove the Helper


      // must return false. (see stack for example)
      return false;
    };

    bool is_valid() {
      return op_->helper() == this;
    };

    bool remove(std::atomic<Node *> *address) {
      // TODO: implement this function, determine the logical value of the helper and then replace.

    };

    Op * const op_;
    Node * newValue_;
    Node * expValue_;
    void expValue(Node *e) { expValue_ = e; };
    void newValue(Node *e) { newValue_ = e; };
  };

  Op(Queue<T> *queue) : queue_(queue) {};

  ~Op() {
    // TODO: implement, free helper

  };

  void fail() {
    associate(fail_val_);
  };

  bool notDone() {
    return helper() == nullptr;
  };

  bool on_is_watched() {
    // TODO: implement this function, it should return if the helper object is watched

  };

  std::atomic<Helper *> helper_{nullptr};
  Helper * helper() { return helper_.load(); };

  bool associate(Helper *h) {
    Helper * temp = nullptr;
    bool res = helper_.compare_exchange_strong(temp, h);
    return res || helper() == h;
  };

  static constexpr Helper * fail_val_ = reinterpret_cast<Helper *>(0x1L);

  Queue<T> * const queue_ {nullptr};
};


template<typename T>
class Queue<T>::DequeueOp : public Queue<T>::Op {
 public:
  DequeueOp(Queue<T> *queue)
    : Op(queue) {};

  bool result(Accessor &access) {
    // TODO: implement this function

  };

  void help_complete() {
    // TODO: implement this function based on your LF code.
    // write a blurb in comments about why this lock-free code behaves wait-free as a result of the announcement table and how it affects new operation creation
    // // Maybe helpful: reinterpret_cast<Node *>
    typename Queue<T>::Op::Helper *helper = new typename Queue<T>::Op::Helper(this);


    while (/* todo */) {
      Accessor access;
      // TODO implement
      // Dont forget the case where the queue is empty

    }  // End while
  };

  T value_;
  // DISALLOW_COPY_AND_ASSIGN(DequeueOp);
};  // class


template<typename T>
class Queue<T>::EnqueueOp : public Queue<T>::Op {
 public:
  EnqueueOp(Queue<T> *queue, T &value)
    : Op(queue) {
      value_ = value;
  };

  void help_complete() {
    // TODO: implement this function based on your LF code.
    // Maybe helpful: reinterpret_cast<Node *>
    typename Queue<T>::Op::Helper *helper = new typename Queue<T>::Op::Helper(this);

    while (/* todo*/) {
    // TODO implement based on the Lock-free code

    }
  };

  T value_;
  // DISALLOW_COPY_AND_ASSIGN(EnqueueOp);
};  // class

}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // __TERVEL_CONTAINERS_WF_LINKED_LIST_QUEUE_OP_RECORD_H_