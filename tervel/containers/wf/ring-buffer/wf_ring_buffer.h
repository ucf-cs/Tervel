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
#ifndef TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_H_
#define TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_H_

#include <atomic>
#include <assert.h>
#include <cstddef>
#include <memory>

namespace tervel {
namespace containers {
namespace wf {

template<typename T>
class RingBuffer {
  static const uintptr_t reserved_lsb = 3;
  static const uintptr_t mark_lsb = 0x1;
  static const uintptr_t emptynode_lsb = 0x2;
  static const uintptr_t oprec_lsb = 0x3;

  static_assert(sizeof(T) == sizeof(uintptr_t) && sizeof(uintptr_t) == sizeof(uint64_t), " Pointers muse be 64 bits");

 public:
  RingBuffer(size_t capacity);

  bool isFull();
  bool isEmpty();
  bool enqueue(T value);
  bool dequeue(T &value);
 private:
  const size_t capacity_;
  std::atomic<uint64_t> head {0};
  std::atomic<uint64_t> tail {0};
  std::unique_ptr<std::atomic<uintptr_t>[]> array_;

  uintptr_t DataNode(T value, uint64_t seqid);
  uintptr_t EmptyNode(uint64_t seqid);
  uintptr_t MarkNode(uint64_t node);
  uintptr_t getEmptyNodeSeqId(uint64_t emptyNode);
  uintptr_t getDataNodeSeqId(uint64_t emptyNode);
  uintptr_t isEmptyNode(uint64_t p);
  uintptr_t isDataNode(uint64_t p);
  uintptr_t isDelayedMarked(uint64_t p);
  uintptr_t nextSeqId(uint64_t seqid);
  uintptr_t getPos(uint64_t seqid);
  uint64_t nextHead();
  uint64_t nextTail();
  void backoff(std::atomic<uintptr_t> *address, uintptr_t *val);
  void backoff() {} ;
};  // class RingBuffer<Value>



}  // namespace wf
}  // namespace containers
}  // namespace tervel

#include <tervel/containers/wf/ring-buffer/wf_ring_buffer_imp.h>

#endif  // TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_H_