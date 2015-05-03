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

#ifndef WF_RINGBUFFER_API_H_
#define WF_RINGBUFFER_API_H_

#include <string>
#include <tervel/containers/wf/ring-buffer/ring_buffer.h>
#include <tervel/util/info.h>
#include <tervel/util/thread_context.h>
#include <tervel/util/tervel.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hp_list.h>

template<class Value>
class TestClass {
 public:
  class WrapperType;

  typedef tervel::containers::wf::RingBuffer<WrapperType *> container_t;

  class WrapperType : public container_t::Value {
   public:
    WrapperType(Value x) : x_(x) {};
    Value value() { return x_; };
    std::string toString() {
      // uint64_t x = (thread_id << 56) | loop_count;
      uint64_t loop = x_ & 0x00FFFFFFFFFFFFFF;
      uint64_t tid = x_ >> 56;
      return "TID: " + std::to_string(tid) + " LC: " + std::to_string(loop);
    }
   private:
    const Value x_;
  };

  void sanity_check();

  TestClass(size_t num_threads, size_t capacity) {
    tervel_obj = new tervel::Tervel(num_threads);
    attach_thread();
    container = new container_t(capacity);
  }

  ~TestClass() {
    std::string temp = container->debug_string();
    std::cout << temp << std::endl;
    delete container;
  }

  std::string toString() {
    return "WF RingBuffer";
  };

  void attach_thread() {
    tervel::ThreadContext* thread_context __attribute__((unused));
    thread_context = new tervel::ThreadContext(tervel_obj);
  };

  void detach_thread() {};

  bool enqueue(Value value) {
    WrapperType *temp = new WrapperType(value);
    bool res = container->enqueue(temp);
    return res;
  };

  bool dequeue(Value& value) {
    WrapperType *temp;
    bool res = container->dequeue(temp);
    if (res) {
      value = temp->value();
    }
    return res;
  };

 private:
  tervel::Tervel* tervel_obj;
  container_t *container;
};

#endif  // WF_RINGBUFFER_API_H_
