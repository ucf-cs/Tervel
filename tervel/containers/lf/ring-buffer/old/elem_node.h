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
#ifndef TERVEL_WFRB_ELEMNODE_H_
#define TERVEL_WFRB_ELEMNODE_H_

#include <tervel/containers/wf/ring-buffer/node.h>
#include <tervel/containers/wf/ring-buffer/buffer_op.h>
#include <tervel/util/memory/hp/hazard_pointer.h>


namespace tervel {
namespace containers {
namespace wf {
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

  ~ElemNode<T>() {}

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


}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_RINGBUFFER_ELEMNODE_H_
