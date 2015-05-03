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
#ifndef TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_ENQUEUEOP_IMP_H_
#define TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_ENQUEUEOP_IMP_H_

#include <tervel/containers/lf/ring-buffer/enqueue_op.h>

namespace tervel {
namespace containers {
namespace lf {

template<typename T>
void
RingBuffer<T>::EnqueueOp::
help_complete() {
  int64_t tail = this->rb_->getTail();
  while(this->BufferOp::notDone()) {
    if (this->rb_->isFull(tail, this->rb_->getHead())) {
      this->fail();
      return;
    }
    int64_t seqid = tail++;
    uint64_t pos = this->rb_->getPos(seqid);
    uintptr_t val;

    while(this->BufferOp::notDone()) {
      if (!this->rb_->readValue(pos, val)) {
        continue;
      }

      int64_t val_seqid;
      bool val_isValueType;
      bool val_isDelayedMarked;
      this->rb_->getInfo(val, val_seqid, val_isValueType, val_isDelayedMarked);


      if (val_seqid > tail) {
        // We are lagging, so iterate until we find a matching seqid.
        break;
      } else if (val_isDelayedMarked) {
        // We don't want a delayed marked anything, too complicated, let some
        // one else deal with it.
        break;
      } else if (val_isValueType) {
        // it is a valueType, with seqid <= to the one we are working with...
        // skip?
        break;
      } else {
        // Its an EmptyType with a seqid <= to the one we are working with
        // so lets take it!
        Helper * helper = new Helper(this, val);
        uintptr_t helper_int = Helper::HelperType(helper);

        bool res = this->rb_->array_[pos].compare_exchange_strong(val, helper_int);
        if (res) {
          // Success!
          // The following line is not hacky if you ignore the function name...
          // it associates and then removes the object.
          // It is also called by the memory protection watch function...
          std::atomic<void *> *temp1;
          temp1 = reinterpret_cast<std::atomic<void *> *>(&(this->rb_->array_[pos]));
          void *temp2 = reinterpret_cast<void *>(helper_int);
          helper->on_watch(temp1, temp2);
          if (!helper->valid()) {
            helper->safe_delete();
          }

          return;  // Op is Done!
        } else {
          // Failure :(
          delete helper;
          continue;  // re-examine position on the next loop
        }
      }
    }
  }
}

template<typename T>
void*
RingBuffer<T>::EnqueueOp::
associate(Helper *h) {
  bool res = BufferOp::privAssociate(h);
  uintptr_t new_val = h->old_value_;
  if (res) {
    int64_t ev_seqid = reinterpret_cast<int64_t>(this) * -1;
    int64_t seqid = this->rb_->getEmptyTypeSeqId(h->old_value_);
    value_->atomic_change_seqid(ev_seqid, seqid);

    uintptr_t temp = reinterpret_cast<uintptr_t>(value_);
    assert((temp & clear_lsb) == 0 && " reserved bits are not 0?");
    new_val = temp;
  }
  // Now we need to ensure the sequence counter does not false report
  // empty
  if (result()){
    int64_t seqid = value_->func_seqid() + 1;
    int64_t temp = this->rb_->getTail();
    while (temp < seqid) {
      if (this->rb_->casTail(temp, seqid))
        break;
    }
  }
  return reinterpret_cast<void *>(new_val);

}


template<typename T>
bool
RingBuffer<T>::EnqueueOp::
result() {
  Helper * h;
  if (BufferOp::isFail(h)) {
    return false;
  } else {
    return true;
  }
};

}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_ENQUEUEOP_IMP_H_