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

#ifndef WF_VECTOR_API_H_
#define WF_VECTOR_API_H_
#include <string>

#include <tervel/containers/wf/vector/vector.hpp>
#include <tervel/util/info.h>
#include <tervel/util/thread_context.h>
#include <tervel/util/tervel.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hp_list.h>

template<typename T>
class TestClass {
 public:
  TestClass(size_t num_threads, size_t capacity) {
    tervel_obj = new tervel::Tervel(num_threads);
    attach_thread();
    container = new tervel::containers::wf::vector::Vector<T>(capacity);
  }

  std::string toString() {
    return "WF Vector";
  };

  void attach_thread() {
    tervel::ThreadContext* thread_context __attribute__((unused));
    thread_context = new tervel::ThreadContext(tervel_obj);
  };

  void detach_thread() {};

  bool at(size_t idx, T &value) {
    return container->at(idx, value);
  };

  bool cas(size_t idx, T &expValue, T newValue) {
    return container->cas(idx, expValue, newValue);
  };

  size_t push_back(T value) {
    // return container->push_back_only(value);
    return container->push_back(value);
  };

  bool pop_back(T &value) {
    return container->pop_back(value);
  };

  size_t size() {
    return container->size();
  };

 private:
  tervel::Tervel* tervel_obj;
  tervel::containers::wf::vector::Vector<T> *container;
};

#endif  // WF_VECTOR_API_H_
