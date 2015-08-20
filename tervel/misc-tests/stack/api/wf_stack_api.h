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

#ifndef WF_STACK_API_H_
#define WF_STACK_API_H_

#include <string>
#include <tervel/containers/wf/stack/stack.h>
#include <tervel/util/info.h>
#include <tervel/util/thread_context.h>
#include <tervel/util/tervel.h>

template<class Value>
class TestClass {
 public:
  typedef tervel::containers::wf::Stack<Value> container_t;


  TestClass(size_t num_threads) {
    tervel_obj = new tervel::Tervel(num_threads);
    attach_thread();
    container = new container_t();
  }

  ~TestClass() {
    delete container;
  }

  std::string toString() {
    return "WF Stack";
  };

  void attach_thread() {
    tervel::ThreadContext* thread_context __attribute__((unused));
    thread_context = new tervel::ThreadContext(tervel_obj);
  };

  void detach_thread() {};

  bool top(Value &value) {
    bool res = container->top(value);
    return res;
  };

  bool pop(Value &value) {
    bool res = container->pop(value);
    return res;
  };

  bool push(Value value) {
    bool res = container->push(value);
    return res;
  };

 private:
  tervel::Tervel* tervel_obj;
  container_t *container;
};

#endif  // WF_STACK_API_H_
