#ifndef TERVEL_WFRB_EMPTYNODE_H_
#define TERVEL_WFRB_EMPTYNODE_H_

#include <tervel/containers/wf/ring-buffer/node.h>


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

  ~EmptyNode<T>() {}

  // REVIEW(steven) missing description
  bool is_EmptyNode() { return true; }

  // REVIEW(steven) missing description
  bool is_ElemNode() { return false; }
};  // EmptyNode class

}  // namespace wf_ring_buffer
}  // namespace tervel
#endif  // TERVEL_WFRB_EMPTYNODE_H_
