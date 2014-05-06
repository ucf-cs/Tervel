#ifndef TERVEL_WFRB_NODE_H_
#define TERVEL_WFRB_NODE_H_

#include "tervel/util/info.h"
#include "tervel/util/util.h"

namespace tervel {
namespace wf_ring_buffer {
/**
 * TODO(ATB) insert class description
 */
template<class T>
class Node : public util::Descriptor {
 public:
  explicit Node<T>(long seq)
      : seq_(seq) {}

  ~Node<T>() {
    // TODO call OpRec safeFree(true) if not null
  }

  using util::Descriptor::on_watch;
  bool on_watch(std::atomic<void*> *address, void *value) {
    typedef util::memory::HazardPointer::SlotID t_SlotID;
    bool success = util::memory::hp::HazardPointer::watch(t_SlotID::SHORTUSE,
                                                          buffer_op,
                                                          address,
                                                          value);
    if (success) {
      Node<T> *curr_node = buffer_->
    }
  }

  using util::Descriptor::on_is_watched;
  bool on_is_watched() {

  }

  Seq() { return seq_; }

 private:
  long seq_;
  OpRecord op_rec_ {nullptr};
};  // Node class

/**
 * TODO(ATB) insert class description: Node without contains a value; dequeued
 */
template<class T>
class NullNode : public Node {
 public:
  explicit NullNode<T>(long seq)
      : seq_(seq) {}

  ~NullNode<T>() {
    // TODO call OpRec safeFree(true) if not null
  }

 //private:

};  // NullNode class

/**
 * TODO(ATB) insert class description: Node that contains a value; enqueued
 */
template<class T>
class ElemNode : public Node {
 public:
  explicit ElemNode<T>(T val, long seq)
      : val_(val)
      , seq_(seq) {}

  ~ElemNode<T>() {
    // TODO call OpRec safeFree(true) if not null
  Val() { return val_; }

 private:
  T val_;

  }

 //private:

};  // ElemNode class

}  // namespace wf_ring_buffer
}  // namespace tervel
#endif  // TERVEL_WFRB_NODE_H_
