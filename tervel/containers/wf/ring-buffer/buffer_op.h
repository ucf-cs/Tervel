/*
The MIT License (MIT)

Copyright (c) 2015 University of Central Florida's Computer Software Engineering
Scalable & Secure Systems (CSE - S3) Lab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef TERVEL_WFRB_BUFFEROP_H
#define TERVEL_WFRB_BUFFEROP_H


#include <atomic>

#include <tervel/util/progress_assurance.h>
#include <tervel/containers/wf/ring-buffer/elem_node.h>
#include <tervel/containers/wf/ring-buffer/dequeue_op.h>
#include <tervel/containers/wf/ring-buffer/enqueue_op.h>
#include <tervel/containers/wf/ring-buffer/wf_ring_buffer.h>


namespace tervel {
namespace wf_ring_buffer {


template<class T>
class RingBuffer;
template<class T>
class ElemNode;
template<class T>
class EnqueueOp;
template<class T>
class DequeueOp;
/**
 * // REVIEW(steven) missing descriptionn of class
 */
template <class T>
class BufferOp : public util::OpRecord {
 public:
  explicit BufferOp<T>(RingBuffer<T> *buffer)
      : buffer_(buffer) {}

  ~BufferOp<T>() {}

  // REVIEW(steven) missing description
  void try_set_failed() {
    ElemNode<T> *null_node = nullptr;
    this->helper_.compare_exchange_strong(null_node, FAILED);
  }

  /**
   * [associate description]
   * @param  node    [description]
   * @param  address [description]
   * @return whether or not this function changed the value at the address
   */
  virtual bool associate(ElemNode<T> *node, std::atomic<Node<T>*> *address) = 0;

  // REVIEW(steven) missing description
  virtual bool result() {
    assert(helper_.load());
    return helper_.load() != FAILED;
  }

  // REVIEW(steven) missing description
  virtual void help_complete() {
    assert(false);
  };

 protected:
  RingBuffer<T> *buffer_;
  std::atomic<ElemNode<T> *> helper_ {nullptr};
  static constexpr ElemNode<T> *FAILED = reinterpret_cast<ElemNode<T> *>(0x1L);

  friend class RingBuffer<T>;
  friend class DequeueOp<T>;
  friend class ElemNode<T>;
  friend class EnqueueOp<T>;
};  // BufferOp class

}  // namespace wf_ring_buffer
}  // namespace tervel

#endif  // TERVEL_WFRB_BUFFEROP_H
