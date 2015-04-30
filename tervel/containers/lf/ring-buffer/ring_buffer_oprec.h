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
#ifndef TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_OPREC_H_
#define TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_OPREC_H_

#include <atomic>
#include <assert.h>
#include <cstddef>
#include <memory>
#include <thread>
#include <string>

#include <tervel/util/info.h>
#include <tervel/util/descriptor.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/hp/hazard_pointer.h>
#include <tervel/util/memory/rc/descriptor_util.h>

namespace tervel {
namespace containers {
namespace lf {


template<typename T>
class RingBuffer<T>::BufferOp{
 public:
  BufferOp(RingBuffer<T> *rb)
    : rb_(rb) {}
  ~BufferOp() {
    Helper *h = helper_.load();
    if (h != nullptr && h != BufferOp::fail) {
      delete h;
    }
  };

  bool associate(Helper *h) {
    Helper *temp = nullptr;
    bool res = helper_.compare_exchange_strong(temp, h);
    if (res || temp == nullptr) { // success
      return true;
    } else { // fail
      return false;
    }
  }

  bool valid(Helper * h) {
    return helper_.load() == h;
  }

  void fail() {
    Helper *temp = nullptr;
    helper_.compare_exchange_strong(temp, fail_val_);
  }

  void notDone() {
    return helper_.load() == nullptr;
  }

  // on_is_watched()
  // This function is not needed because helpers will be removed before removing
  // the watch on the op record
  friend class RingBuffer<T>::EnqueueOp;
  friend class RingBuffer<T>::DequeueOp;
 private:
  const Helper * fail_val_{static_cast<Helper *>(0x1)};
  const RingBuffer<T> *rb_;
  std::atomic<Helper *> helper_{nullptr};
};

template<typename T>
class RingBuffer<T>::Helper { // TODO(steven): extend hp descriptor
 public:
  Helper(BufferOp *op, uintptr_t old_value)
   : op_(op)
   , old_value_(old_value) {}
  ~Helper() {}

  bool on_watch(std::atomic<void *> *address, void *expected) {
    // TODO(steven): watch op_.
    void *val = associate();
    address->compare_exchange_strong(expected, val);
    return false;
  }

  void * associate() {
    return op_->associate(this);
  }

  bool valid() {
    return op_->valid(this);
  }

  friend class RingBuffer::EnqueueOp;
 private:
  const BufferOp *op_;
  const uintptr_t old_value_;

};

template<typename T>
class RingBuffer<T>::EnqueueOp: public BufferOp {
 public:
  EnqueueOp(RingBuffer<T> *rb, T value)
    : BufferOp(rb)
    , value_(value) {
      int64_t seqid = reinterpret_cast<int64_t>(this) * -1;
      value_->func_seqid(seqid);
    }

  void * associate(Helper *h) {
    bool res = BufferOp::associate(h);
    if (res) {
      int64_t ev_seqid = reinterpret_cast<int64_t>(this) * -1;
      int64_t seqid = this->rb_->getEmptyNodeSeqId(h->old_value_);
      value_->atomic_change_seqid(ev_seqid, seqid);

      uintptr_t res = reinterpret_cast<uintptr_t>(value_);
      assert((res & clear_lsb) == 0 && " reserved bits are not 0?");
      return res;
    } else {
      return reinterpret_cast<void *>(h->old_value_);
    }

  }

  void help_complete();

 private:
  T value_;
};

template<typename T>
class RingBuffer<T>::DequeueOp: public BufferOp {
 public:
  DequeueOp(RingBuffer<T> *rb)
    : BufferOp(rb) {}

  void help_complete();

};

template<typename T>
void RingBuffer<T>::EnqueueOp::help_complete() {
  int64_t tail = getTail();
  while(this->BufferOp::notDone()) {
    if (this->rb_->isFull(tail, this->rb_->getHead())) {
      this->fail();
      return;
    }
    int64_t seqid = tail++;
    uint64_t pos = this->rb_->getPos(seqid);
    uintptr_t val;
    if (!this->rb_->readValue(pos, val)) {
      continue;
    }

    int64_t val_seqid;
    bool val_isValueType;
    bool val_isDelayedMarked;
    getInfo(val, val_seqid, val_isValueType, val_isDelayedMarked);


    if (val_seqid > tail) {
      // We are lagging, so iterate until we find a matching seqid.
      continue;
    } else if (val_isDelayedMarked) {
      // We don't want a delayed marked anything, too complicated, let some
      // one else deal with it.
      continue;
    } else if (val_isValueType) {
      // it is a valueType, with seqid <= to the one we are working with...
      // skip?
      continue;
    } else {
      // Its an EmptyNode with a seqid <= to the one we are working with
      // so lets take it!
      Helper * helper = new Helper(this, val);
      uintptr_t helper_int = this->rb_->MakeHelper(helper);

      bool res = this->rb_->array_[pos].compare_exchange_strong(val, helper_int);
      if (res) {
        // Success!
        // The following line is not hacky if you ignore the function name...
        // it associates and then removes the object.
        // It is also called by the memory protection watch function...
        helper->on_watch(&(this->rb_->array_[pos]), helper_int);
        if (!helper->valid()) {
          helper->safeDelete();
        }
      } else {
        // Failure :(
        delete helper;
        tail--;  // re-examine position on the next loop
      }
    }

  }
}

template<typename T>
void RingBuffer<T>::DequeueOp::help_complete() {
  int64_t head = getHead();
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
    getInfo(val, val_seqid, val_isValueType, val_isDelayedMarked);


    if (val_seqid > head) {
      // We are lagging, so iterate until we find a matching seqid.
      continue;
    } else if (val_isValueType) {
      // it is a valueType, with seqid <= to the one we are working with...
      // so we take it or try to any way...
      // TODO(steven): code this logic...
    } else {
      // Its an EmptyNode with a seqid <= to the one we are working with
      // so lets fuck shit up and set it delayed mark that will show them...
      // but it is the simplest way to ensure nothing gets enqueued at this pos
      // which allows us to keep fifo.
      // If something did get enqueued then, it will be marked and we will check
      // it on the next round
      if (val_isDelayedMarked) {
        // its already been marked, so move on to the next pos;
        continue;
      } else {
        atomic_delay_mark(pos);
        head--;  // re-read/process the value
      }



    }

  }
}
}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_OPREC_H_