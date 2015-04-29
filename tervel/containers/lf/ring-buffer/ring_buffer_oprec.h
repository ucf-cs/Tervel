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
namespace ringbuffer {

template<typename T>
class DequeueOp: public tervel::util::OpRecord {
  static const Helper *fail_const = static_cast<Helper *>(0x1);
 public:
  DequeueOp(RingBuffer<T> *rb)
    : ring_buffer_(rb) {


  }

  void fail() {
    Helper * temp = nullptr;
    helper_.compare_exchange_strong(temp, fail_const);
  };

  void notDone() {
    return helper.load() == nullptr;
  }


 private:
  RingBuffer<T> *ring_buffer_{nullptr};
  std::atomic<Helper *> helper_{nullptr};


};  // class DequeueOp

void DequeueOp::
help_complete() {
  while(notDone()) {
    if (rb->isEmpty()) {
      this->fail();
      return;
    }

    int64_t seqid = rb->getHead();
    uint64_t pos = rb->getPos(seqid);
    uintptr_t val = rb->array_[pos].load();
    uintptr_t new_value = rb->EmptyNode(rb->nextSeqId(seqid));

    while (notDone()) {
      int64_t val_seqid;
      bool val_isValueType;
      bool val_isDelayedMarked;
      rb->getInfo(val, val_seqid, val_isValueType, val_isDelayedMarked);

      if (val_seqid > seqid) {
        break;
      }
      if (val_isValueType) {
        if (val_seqid == seqid) {
          if (val_isDelayedMarked) {
            new_value = rb->MarkNode(new_value);
            assert(isDelayedMarked(new_value));
          }
          value = getValueType(val);

          Helper *helper = new Helper(this, value, new_value);
          uintptr_t helper_int = MakeHelper(helper);
          if (rb->array_[pos].compare_exchange_strong(val, helper_int)) {
            helper->on_watch(&(rb->array_[pos]), helper_int);
            if (this->helper_.load() != helper) {
              delete helper;
            }
            return;
          } else {
            delete helper;
          }

        } else { // val_seqod < seqid
          if (backoff(&array_[pos], val)) {
            // value changed
            continue; // process the new value.
          }
          // Value has not changed so lets skip it.
          if (val_isDelayedMarked) {
            // Its marked and the seqid is less than ours so we
            // can skip it safely.
            break;
          } else {
            // we blindly mark it and re-examine the value;
            val = atomic_delay_mark(&array_[pos]);

            continue;
          }
        }
      } else { // val_isEmptyNode
        if (val_isDelayedMarked || !backoff(&array_[pos], val)) {
          // Value has not changed
          if (array_[pos].compare_exchange_strong(val, new_value)) {
            break;
          }
        }
        // Value has changed
        continue;
      }
    }  // inner loop
  } // outer loop.
}

}  // namespace ringbuffer
}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_OPREC_H_