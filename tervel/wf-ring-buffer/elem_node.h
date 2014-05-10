#ifndef TERVEL_WFRB_ELEMNODE_H_
#define TERVEL_WFRB_ELEMNODE_H_

#include "node.h"
#include "tervel/wf-ring-buffer/dequeue_op.h"
#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/hp/hazard_pointer.h"

namespace tervel {
namespace wf_ring_buffer {
/**
 * TODO(ATB) insert class description
 */


template<class T>
class ElemNode : public Node<T> {
 public:
  explicit ElemNode<T>(T val, long seq, util::OpRecord *op_rec = nullptr)
      : Node<T>(val, seq)
      , op_rec_(op_rec) {}
      
      /*val_(val)
      , seq_(seq)
      , op_rec_(op_rec) {}
      */

  ~ElemNode<T>() {
    util::OpRecord *node_op = op_rec_.load();
    if (node_op != nullptr) {
      node_op->safe_delete(true);
    }
  }

  using util::Descriptor::on_watch;
  bool on_watch(std::atomic<void*> *address, void *value) {
    util::OpRecord node_op = op_rec_.load();
    if (node_op != nullptr) {
      typedef util::memory::hp::HazardPointer::SlotID t_SlotID;
      bool success = util::memory::hp::HazardPointer::watch(t_SlotID::SHORTUSE,
                                                            node_op,
                                                            &op_rec_,
                                                            node_op);
      if (success) {
        Node<T> *temp = nullptr;
        bool did_assoc = node_op.helper.compare_exchange_strong(temp, this);
        if (!did_assoc && this != temp) {
          op_rec_.store(nullptr);
        }
      }
    }
    return true;
  }

  using util::Descriptor::on_is_watched;
  bool on_is_watched() {
    util::OpRecord *node_op = op_rec_.load();
    if (node_op != nullptr) {
      return util::memory::hp::HazardPointer::is_watched(node_op);
    }
    return false;
  }

  void associate() {
    Node *assoc_node = op_rec_->node_.load();
    if (assoc_node == nullptr) {
      bool cas_succ = (op_rec_->node_).compare_exchange_strong(assoc_node,
                                                               this);
      if (cas_succ) {
        assoc_node =  this;
      }
    }
    if (assoc_node != this) {
      op_rec_.store(nullptr);
    }
  }

  bool is_owned() { return op_rec_ == nullptr; }

  bool is_EmptyNode() { return false; }
  bool is_NullNode() { return true; }

 private:
  std::atomic<util::OpRecord*> op_rec_ {nullptr};
};  // ElemNode class


}  // namespace wf_ring_buffer
}  // namespace tervel
#endif  // TERVEL_WFRB_ELEMNODE_H_
