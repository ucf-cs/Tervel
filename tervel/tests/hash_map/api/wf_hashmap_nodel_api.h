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

#ifndef WF_HASHMAP_API_H_
#define WF_HASHMAP_API_H_


#include <tervel/containers/wf/hash-map/wf_hash_map_no_delete.h>
#include <tervel/util/info.h>
#include <tervel/util/thread_context.h>
#include <tervel/util/tervel.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hp_list.h>

#ifndef HASHMAP_EXPANSION_RATE
  #define HASHMAP_EXPANSION_RATE 3
#endif

template<class Key, class Value>
class TestClass {
 public:
  typedef typename tervel::containers::wf::HashMap<Key, Value> Map;
  typedef typename tervel::containers::wf::HashMap<Key, Value>::ValueAccessor Accessor;

  TestClass(size_t num_threads, size_t capacity) {
    tervel_obj = new tervel::Tervel(num_threads);
    attach_thread();
    container = new Map(capacity, HASHMAP_EXPANSION_RATE);
  }

  std::string toString() {
    return "WF Hash Map-"+std::to_string(HASHMAP_EXPANSION_RATE);
  }

  void attach_thread() {
    tervel::ThreadContext* thread_context __attribute__((unused));
    thread_context = new tervel::ThreadContext(tervel_obj);
  }

  void detach_thread() {}

  bool find(Key key, Value &value) {
    typename tervel::containers::wf::HashMap<Key, Value>::ValueAccessor va;
    bool res = container->at(key, va);
    if (res) {
      value = *(va.value());
    }
    return res;
  }

  bool insert(Key key, Value value) {
    bool res = container->insert(key, value);
    return res;
  }

  bool update(Key key, Value &value_expected, Value value_new) {
    typename tervel::containers::wf::HashMap<Key, Value>::ValueAccessor va;
    if (container->at(key, va)) {
      std::atomic<Value> *temp = reinterpret_cast<std::atomic<Value> *>(va.value());
      return temp->compare_exchange_strong(value_expected, value_new);
    }
    return false;
  }

  bool remove(Key key) {
    return false;
  }

  size_t size() {
    return container->size();
  }

 private:
  tervel::Tervel* tervel_obj;
  Map *container;
};

#endif  // WF_HASHMAP_API_H_
