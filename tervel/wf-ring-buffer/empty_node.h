#ifndef TERVEL_WFRB_EMPTYNODE_H_
#define TERVEL_WFRB_EMPTYNODE_H_

#include "node.h"

template<class T>
class Node;

namespace tervel {
namespace wf_ring_buffer {
/**
 * TODO(ATB) insert class description
 */
template<class T>
class EmptyNode : public Node<T> {
 public:
  explicit EmptyNode<T>(long seq)
      : Node<T>(reinterpret_cast<T>(nullptr), seq) {}

  ~EmptyNode<T>() {
    // TODO call OpRec safeFree(true) if not null
  }

  // REVIEW(steven) missing description
  bool is_EmptyNode() { return true; }

  // REVIEW(steven) missing description
  bool is_ElemNode() { return false; }

 //private: // just delete the code
};  // EmptyNode class

}  // namespace wf_ring_buffer
}  // namespace tervel
#endif  // TERVEL_WFRB_EMPTYNODE_H_
