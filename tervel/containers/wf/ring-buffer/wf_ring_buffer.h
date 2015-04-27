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
#include <thread>
#include <string>

namespace tervel {
namespace containers {
namespace wf {

template<typename T>
class RingBuffer {
  static const uintptr_t num_lsb = 3;
  static const uintptr_t mark_lsb = 0x1;
  static const uintptr_t emptynode_lsb = 0x2;
  static const uintptr_t oprec_lsb = 0x4;
  static const uintptr_t clear_lsb = 7;

  static_assert(sizeof(T) == sizeof(uintptr_t) && sizeof(uintptr_t) == sizeof(uint64_t), " Pointers muse be 64 bits");

 public:
  class Value {
   public:
    Value() {};
    friend RingBuffer;
   private:
    int64_t func_seqid() {
      assert(seqid_ != -1);
      return seqid_;
    }
    void func_seqid(int64_t s) {
      seqid_ = s;
    }
    int64_t seqid_{-1};
  };

  RingBuffer(size_t capacity);

  bool isFull();
  bool isEmpty();
  bool enqueue(T value);
  bool dequeue(T &value);
  std::string debug_string(uintptr_t val);
  std::string debug_string();

 private:
  const int64_t capacity_;
  std::atomic<int64_t> head {0};
  std::atomic<int64_t> tail {0};
  std::unique_ptr<std::atomic<uintptr_t>[]> array_;

  uintptr_t MarkNode(uint64_t node);
  uintptr_t EmptyNode(int64_t seqid);
  int64_t getEmptyNodeSeqId(uintptr_t val);

  uintptr_t ValueType(T value, int64_t seqid);
  T getValueType(uintptr_t val);
  int64_t getValueTypeSeqId(uintptr_t val);

  void getInfo(uintptr_t val, int64_t &val_seqid,
    bool &val_isValueType, bool &val_isMarked);
  bool isEmptyNode(uintptr_t p);
  bool isValueType(uintptr_t p);
  bool isDelayedMarked(uintptr_t p);

  int64_t nextHead();
  int64_t nextTail();
  int64_t nextSeqId(int64_t seqid);
  int64_t getPos(int64_t seqid);

  bool backoff(std::atomic<uintptr_t> *address, uintptr_t &val);
  void backoff() {
    std::this_thread::yield();
  } ;

  void atomic_delay_mark(std::atomic<uintptr_t> *address, uintptr_t &val);
};  // class RingBuffer<Value>



}  // namespace wf
}  // namespace containers
}  // namespace tervel

#include <tervel/containers/wf/ring-buffer/wf_ring_buffer_imp.h>

#endif  // TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_H_