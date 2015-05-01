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
#ifndef TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_ENQUEUEOP_H_
#define TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_ENQUEUEOP_H_

#include <tervel/containers/lf/ring-buffer/ring_buffer_op.h>

namespace tervel {
namespace containers {
namespace lf {

#include <tervel/containers/lf/ring-buffer/ring_buffer_op.h>


template<typename T>
class RingBuffer<T>::EnqueueOp: public BufferOp {
 public:
  EnqueueOp(RingBuffer<T> *rb, T value)
    : BufferOp(rb)
    , value_(value) {
      int64_t seqid = reinterpret_cast<int64_t>(this) * -1;
      value_->func_seqid(seqid);
    }

  void * associate(Helper *h);
  void help_complete();
  bool result();

 private:
  const T value_;
  DISALLOW_COPY_AND_ASSIGN(EnqueueOp);
};

}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_ENQUEUEOP_H_