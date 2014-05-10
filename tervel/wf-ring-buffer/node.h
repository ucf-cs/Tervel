#ifndef TERVEL_WFRB_NODE_H_
#define TERVEL_WFRB_NODE_H_

#include "tervel/util/descriptor.h"

namespace tervel {
namespace wf_ring_buffer {
/**
 * TODO(ATB) insert class description
 */
template<class T>
class Node : public util::Descriptor {
 public:
  explicit Node<T>(T val, long seq)
      : val_(val)
      , seq_(seq) {}

  ~Node<T>() {

  }

  virtual bool is_ElemNode() = 0;
  virtual bool is_EmptyNode() = 0;

  void* complete(void*, std::atomic<void*>*) {
    assert(false);
  }

  void* get_logical_value() {
    assert(false);
  }


  T val() { return val_; }
  long seq() { return seq_; }

 protected:
  T val_;
  long seq_;
};  // Node class


}  // namespace wf_ring_buffer
}  // namespace tervel
#endif  // TERVEL_WFRB_NODE_H_
