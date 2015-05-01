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
#ifndef TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_HELPER_IMP_H_
#define TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_HELPER_IMP_H_

#include <tervel/containers/lf/ring-buffer/helper.h>

#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hazard_pointer.h>

namespace tervel {
namespace containers {
namespace lf {

template<typename T>
class RingBuffer<T>::Helper : public tervel::util::memory::hp::Element {
 public:
  Helper(BufferOp *op, uintptr_t old_value)
   : op_(op)
   , old_value_(old_value) {}
  ~Helper() {}

template<typename T>
bool
RingBuffer<T>::Helper::
on_watch(std::atomic<void *> *address, void *expected) {
  typedef tervel::util::memory::hp::HazardPointer::SlotID SlotID;
  SlotID pos = SlotID::SHORTUSE2;
  bool res = tervel::util::memory::hp::HazardPointer::watch(pos, op_,
      address, expected);
  if (!res) {
    return false;
  }

  void *val = associate();
  res = address->compare_exchange_strong(expected, val);
  if (!res) {
    // we failed, could be because of delayed mark.
    void *temp = reinterpret_cast<void *>(
        RingBuffer<T>::DelayMarkValue(HelperType(this)));
    if (expected == temp) {
      address->compare_exchange_strong(expected, val);
    }
  }

  #ifdef DEBUG
    expected = address->load();
    assert(expected != reinterpret_cast<void *>(HelperType(this)));
    assert(expected !=
        reinterpret_cast<void *>(
          RingBuffer<T>::DelayMarkValue(HelperType(this))));
  #endif

  return false;
}

template<typename T>
void *
RingBuffer<T>::Helper::
associate() {
  return op_->associate(this);
}

template<typename T>
bool
RingBuffer<T>::Helper::
valid() {
  return op_->valid(this);
}

template<typename T>
uintptr_t
RingBuffer<T>::Helper::
HelperType(Helper *h) {
  uintptr_t res = reinterpret_cast<uintptr_t>(h);
  res = res | RingBuffer<T>::oprec_lsb; // 3LSB now 100
  return res;
}

template<typename T>
bool
RingBuffer<T>::Helper::
isHelperType(uintptr_t val) {
  val = val & RingBuffer<T>::oprec_lsb;
  return (val != 0);
}


template<typename T>
Helper *
RingBuffer<T>::Helper::
getHelperType(uintptr_t val) {
  val = val & (~RingBuffer<T>::oprec_lsb);  // clear oprec_lsb
  val = val & (~RingBuffer<T>::delayMark_lsb);  // clear delayMark_lsb
  return reinterpret_cast<Helper *>(val);
}

}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_HELPER_IMP_H_
