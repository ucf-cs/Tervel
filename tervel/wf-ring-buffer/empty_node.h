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

  ~EmptyNode<T>() {}

  // REVIEW(steven) missing description
  bool is_EmptyNode() { return true; }

  // REVIEW(steven) missing description
  bool is_ElemNode() { return false; }
    
  void print() {
    printf("%ld%ld%ld%ld%ld%ld%ld\n", foo0, foo1, foo2, foo3, foo4, foo5, foo6);
  }

  void math(int val) {
    foo0 = foo1 - val;;
    foo1 = foo2 + val;
    foo2 = foo4 | val;
    foo3 = foo5 & val;
    foo4 = foo3 / val;
    foo5 = foo4 * val;
    foo6 = foo4 * val;
  }

 private:
  long foo0 {10};
  long foo1 {5};
  long foo2 {3959};
  long foo3 {6};
  long foo4 {120};
  long foo5 {107};
  long foo6 {19};
  //int pad __attribute__((aligned(64)));
};  // EmptyNode class

}  // namespace wf_ring_buffer
}  // namespace tervel
#endif  // TERVEL_WFRB_EMPTYNODE_H_
