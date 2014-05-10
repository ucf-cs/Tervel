#ifndef TERVEL_WFRB_BUFFEROP_H
#define TERVEL_WFRB_BUFFEROP_H


#include "tervel/util/progress_assurance.h"
#include "tervel/wf-ring-buffer/elem_node.h"
#include "tervel/wf-ring-buffer/wf_ring_buffer.h"

#include <atomic>

namespace tervel {
namespace wf_ring_buffer {


template<class T>
class RingBuffer;
template<class T>
class ElemNode;

/**
 *
 */
template <class T>
class BufferOp : public util::OpRecord {
  public:
    explicit BufferOp<T>(RingBuffer<T> *buffer, ElemNode<T> *node)
      : buffer_(buffer) 
      , node_(node) {}

  protected:
    RingBuffer<T> *buffer_;
    ElemNode<T> *node_;
    static constexpr ElemNode<T> *FAILED = reinterpret_cast<ElemNode<T> *>(0x1L);
};  // BufferOp class

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_BUFFEROP_H
