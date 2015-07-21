/*
#The MIT License (MIT)
#
#Copyright (c) 2015 University of Central Florida's Computer Software Engineering
#Scalable & Secure Systems (CSE - S3) Lab
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.
#
*/

#ifndef DS_API_H_
#define DS_API_H_


#include <string>
#include <mutex>

template<typename T>
class LockBuffer {
 public:
  LockBuffer(size_t capacity)
   : capacity_(capacity)
   , array_(new T[capacity]()) {};

   size_t size() { return tail_ - head_; };

   bool enqueue(T val) {
    std::lock_guard<std::mutex> lock(lock_);
    if (tail_ < head_ + capacity_) {
      array_[tail_ % capacity_] = val;
      tail_++;
      return true;
    } else {
      return false;
    }
   }

   bool dequeue(T &val) {
    std::lock_guard<std::mutex> lock(lock_);

    if (head_ < tail_) {
      val = array_[head_ % capacity_];
      head_++;
      return true;
    } else {
      return false;
    }
   };
 private:
  int head_ {0};
  int tail_ {0};

  std::mutex lock_;
  size_t size_;
  const size_t capacity_;
  std::unique_ptr<T[]> array_;


};


typedef unsigned char Value;

typedef LockBuffer<Value> container_t;

#include "../../src/main.h"

DEFINE_int32(prefill, 0, "The number elements to place in the buffer on init.");
DEFINE_int32(capacity, 0, "The capacity of the buffer.");

#define DS_DECLARE_CODE \
  container_t *container;

#define DS_DESTORY_CODE

#define DS_ATTACH_THREAD

#define DS_DETACH_THREAD

#define DS_INIT_CODE \
DS_ATTACH_THREAD \
container = new container_t(FLAGS_capacity); \
\
std::default_random_engine generator; \
std::uniform_int_distribution<Value> largeValue(0, UINT_MAX); \
for (int i = 0; i < FLAGS_prefill; i++) { \
  Value x = largeValue(generator) & (~0x3); \
  container->enqueue(x); \
}

#define DS_NAME "LockBuffer"

#define DS_CONFIG_STR \
   "\n" _DS_CONFIG_INDENT "prefill : " + std::to_string(FLAGS_prefill) +"" + \
   "\n" _DS_CONFIG_INDENT "capacity : " + std::to_string(FLAGS_capacity) +""

#define DS_STATE_STR " "

#define OP_RAND \
  /* std::uniform_int_distribution<Value> random(1, UINT_MAX); */ \
  int ecount = 0;


#define OP_CODE \
  MACRO_OP_MAKER(0, { \
    /* Value value = random(); */ \
    Value value = (thread_id << 56) | ecount; \
    opRes =container->enqueue(value); \
  } \
  ) \
 MACRO_OP_MAKER(1, { \
      Value value; \
      opRes = container->dequeue(value); \
    } \
  )

#define DS_OP_NAMES "enqueue", "dequeue"

#define DS_OP_COUNT 2


inline void sanity_check(container_t *container) {};

#endif  // DS_API_H_

