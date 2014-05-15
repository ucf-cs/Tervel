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

  ~ElemNode<T>() {
    /* @Steven, should this really be here?
    if (op_rec_ != nullptr && op_rec_.load() != nullptr) {
      op_rec_.load()->safe_delete();
    }
    */
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
        bool res = node_op->associate(this,
            reinterpret_cast<std::atomic< Node<T> *> *>(address));
        util::memory::hp::HazardPointer::unwatch(t_SlotID::SHORTUSE);
        // Note: the returned result of associate is whether or not the value at
        // the address changed, if it has changed then this watch should fail
        return res;
      }
    }
    return true;
  }


  void clear_op() {
    op_rec_.store(nullptr);
  }
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
