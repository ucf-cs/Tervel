#ifndef TERVEL_WFRB_ELEMNODE_H_
#define TERVEL_WFRB_ELEMNODE_H_

#include "node.h"
#include "tervel/wf-ring-buffer/dequeue_op.h" // REVIEW(steven) not used
#include "tervel/wf-ring-buffer/buffer_op.h"
#include "tervel/util/progress_assurance.h" // REVIEW(steven) not used
#include "tervel/util/memory/hp/hazard_pointer.h"

namespace tervel {
namespace wf_ring_buffer {
/**
 * TODO(ATB) insert class description
 */

// REVIEW(steven) No Space between comment and class
template<class T>
class ElemNode : public Node<T> {
 public:
  explicit ElemNode<T>(T val, long seq, BufferOp<T> *op_rec = nullptr)
      : Node<T>(val, seq)
      , op_rec_(op_rec) {}

      /*val_(val) // REVIEW(steven) just delete the code
      , seq_(seq)
      , op_rec_(op_rec) {}
      */

  // REVIEW(steven) missing description
  ~ElemNode<T>() {
    BufferOp<T> *node_op = op_rec_.load();
    if (node_op != nullptr) {
      node_op->safe_delete(true);
    }
  }

  // REVIEW(steven) missing description
  using util::Descriptor::on_watch;
  bool on_watch(std::atomic<void*> *address, void *value) {
    BufferOp<T> *node_op = op_rec_.load();

    if (node_op != nullptr) {
      typedef util::memory::hp::HazardPointer::SlotID t_SlotID;
      tervel::util::memory::hp::Element *temp_op = reinterpret_cast<
          tervel::util::memory::hp::Element *>(node_op);
      std::atomic<void *> * temp_address = reinterpret_cast<
          std::atomic<void *>* >(&op_rec_);
      void *temp_expected = reinterpret_cast<void *>(node_op);

      bool success = util::memory::hp::HazardPointer::watch(t_SlotID::SHORTUSE,
                                                            temp_op,
                                                            temp_address,
                                                            temp_expected);
      if (success) {
        // REVIEW(steven) you should use associate function instead
        Node<T> *temp = nullptr;
        bool did_assoc = node_op->helper_.compare_exchange_strong(temp, this);
        if (!did_assoc && this != temp) {
          op_rec_.store(nullptr);
        }
        // REVIEW(steven) this is the correct logic for dequeue op, but not for
        // enqueue op. Enqueue op if the association fails the node should be
        // replaced by an EmpytNode
        // this can be accomplished by moving the above code/association
        // function into the operation records. you will need to pass the
        // address/value to be able to remvoe the node as well
      }

    }
    return true;
  }

  // REVIEW(steven)  missing discription
  using util::Descriptor::on_is_watched;
  bool on_is_watched() {
    BufferOp<T> *node_op = op_rec_.load();
    if (node_op != nullptr) {
      return util::memory::hp::HazardPointer::is_watched(node_op);
    }
    return false;
  }

  // REVIEW(steven) missing description
  void associate() {
    Node<T> *assoc_node = op_rec_->helper_.load();
    if (assoc_node == nullptr) {
      bool cas_succ = (op_rec_->helper_).compare_exchange_strong(assoc_node,
                                                               this);
      if (cas_succ) {
        assoc_node =  this;
      }
    }
    if (assoc_node != this) {
      op_rec_.store(nullptr);
    }
  }

  void delete_op() {
    op_rec_.store(nullptr);
  }

  // REVIEW(steven) missing description
  // REVIEW(steven) should be op_rec_.load()
  bool is_owned() { return op_rec_ == nullptr; }

  // REVIEW(steven) missing description
  bool is_EmptyNode() { return false; }

  // REVIEW(steven) missing description
  bool is_ElemNode() { return true; }

 private:
  std::atomic<BufferOp<T>*> op_rec_ {nullptr};
};  // ElemNode class


}  // namespace wf_ring_buffer
}  // namespace tervel
#endif  // TERVEL_WFRB_ELEMNODE_H_
