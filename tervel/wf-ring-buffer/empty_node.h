#ifndef TERVEL_WFRB_NODE_H_
#define TERVEL_WFRB_NODE_H_

#include "node.h"

namespace tervel {
namespace wf_ring_buffer {
/**
 * TODO(ATB) insert class description
 */
template<class T>
class EmptyNode : public Node {
 public:
  explicit EmptyNode<T>(long seq)
      : val_(nullptr)
      , seq_(seq) {}

  ~EmptyNode<T>() {
    // TODO call OpRec safeFree(true) if not null
  }

  bool is_EmptyNode() { return true; }
  bool is_NullNode() { return false; }

 //private:
};  // EmptyNode class

}  // namespace wf_ring_buffer
}  // namespace tervel
#endif  // TERVEL_WFRB_NODE_H_
