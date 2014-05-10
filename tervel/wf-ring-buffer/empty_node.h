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
      : Node<T>(nullptr, seq) {}

  ~EmptyNode<T>() {
    // TODO call OpRec safeFree(true) if not null
  }

  bool is_EmptyNode() { return true; }
  bool is_NullNode() { return false; }

 //private:
};  // EmptyNode class

}  // namespace wf_ring_buffer
}  // namespace tervel
#endif  // TERVEL_WFRB_EMPTYNODE_H_
