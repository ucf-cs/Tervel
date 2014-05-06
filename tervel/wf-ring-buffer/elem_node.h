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
class ElemNode : public Node {
 public:
  explicit ElemNode<T>(T val, long seq)
      : val_(val)
      , seq_(seq) {}

  ~ElemNode<T>() {
    // TODO call OpRec safeFree(true) if not null
  }

  using util::Descriptor::on_watch;
  bool on_watch(std::atomic<void*> *address, void *value) {
    OpRecord node_op = op_rec_.load();
    if (node_op != nullptr) {
      typedef util::memory::HazardPointer::SlotID t_SlotID;
      bool success = util::memory::hp::HazardPointer::watch(t_SlotID::SHORTUSE,
                                                            node_op,
                                                            &op_rec_,
                                                            node_op);
      if (success) {
        Node *temp = nullptr;
        bool did_assoc = node_op->helper.compare_exchange_strong(temp, this);
        if (!did_assoc && this != temp) {
          op_rec_.store(nullptr);
        }
      }
    }
    return true;
  }

  using util::Descriptor::on_is_watched;
  bool on_is_watched() {
    OpRecord *node_op = op_rec_.load();
    if (node_op != nullptr) {
      return util::memory::hp::HazardPointer::iswatched(node_op);
    }
    return false;
  }

  bool is_owned() { return op_rec_  == nullptr; }

  bool is_EmptyNode() { return false; }
  bool is_NullNode() { return true; }

 private:
  atomic<OpRecord*> op_rec_ {nullptr};
};  // ElemNode class


}  // namespace wf_ring_buffer
}  // namespace tervel
#endif  // TERVEL_WFRB_NODE_H_
