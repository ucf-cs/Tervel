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
#ifndef TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_DEQUEUEOP_IMP_H_
#define TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_DEQUEUEOP_IMP_H_

#include <tervel/containers/lf/ring-buffer/dequeue_op.h>

namespace tervel {
namespace containers {
namespace lf {

template<typename T>
void RingBuffer<T>::DequeueOp::help_complete() {
  int64_t head = this->rb_->getHead();
  while(this->BufferOp::notDone()) {
    if (this->rb_->isEmpty(head, this->rb_->getTail())) {
      this->fail();
      return;
    }
    int64_t seqid = head++;
    uint64_t pos = this->rb_->getPos(seqid);
    uintptr_t val;
    if (!this->rb_->readValue(pos, val)) {
      continue;
    }

    int64_t val_seqid;
    bool val_isValueType;
    bool val_isDelayedMarked;
    this->rb_->getInfo(val, val_seqid, val_isValueType, val_isDelayedMarked);


    if (val_seqid > head) {
      // We are lagging, so iterate until we find a matching seqid.
      continue;
    } else if (val_isValueType) {
      // it is a valueType, with seqid <= to the one we are working with...
      // so we take it or try to any way...

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
      } else {
        // Failure :(
        delete helper;
        head--;  // re-examine position on the next loop
      }

    } else {
      // Its an EmptyType with a seqid <= to the one we are working with
      // so lets mess shit up and set it delayed mark that will show them...
      // but it is the simplest way to ensure nothing gets enqueued at this pos
      // which allows us to keep fifo.
      // If something did get enqueued then, it will be marked and we will check
      // it on the next round
      if (val_isDelayedMarked) {
        // its already been marked, so move on to the next pos;
        continue;
      } else {
        this->rb_->atomic_delay_mark(pos);
        head--;  // re-read/process the value
      }
    }  // its an EmptyType
  }  // while notDone
}  // void RingBuffer<T>::DequeueOp::help_complete()

template<typename T>
void *
RingBuffer<T>::DequeueOp::
associate(Helper *h) {
  bool res = BufferOp::privAssociate(h);
  uintptr_t temp = h->old_value_;
  if (res) {

    int64_t seqid = RingBuffer<T>::getEmptyTypeSeqId(temp);
    int64_t next_seqid = BufferOp::rb_->nextSeqId(seqid);
    uintptr_t new_value = RingBuffer<T>::EmptyType(next_seqid);
    if (RingBuffer<T>::isDelayedMarked(temp)) {
      new_value = RingBuffer<T>::DelayMarkValue(temp);
    }

    return reinterpret_cast<void *>(new_value);
  } else {
    return reinterpret_cast<void *>(temp);
  }

}

template<typename T>
bool RingBuffer<T>::DequeueOp::
result(T &val) {
  Helper * h;
  if (BufferOp::isFail(h)) {
    return false;
  } else {
    uintptr_t temp = h->old_value_;
    val = getValueType(temp);
    return true;
  }
};

}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_DEQUEUEOP_IMP_H_