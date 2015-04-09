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

#ifndef WF_HASHMAP_OLD_API_H_
#define WF_HASHMAP_OLD_API_H_

#include <atomic>
#include "/home/damian/archive/WFHashTable/WaitFreeHashTable.cpp"

thread_local long thread_id;

template<class Key, class Value>
class TestClass {
 public:
  std::atomic<long> thread_counter;

  TestClass(size_t num_threads, size_t capacity) {
    int pow_of_two = 0;

    while (capacity != 1) {
      capacity = capacity / 2;
      pow_of_two++;
    }
    if (pow_of_two < 6)
      pow_of_two = 6;

    container = new WaitFreeHashTable<Key, Value>(pow_of_two, num_threads);
    thread_counter.store(0);
  }

  std::string name() {
    return "Old WF Hash Map-"+std::to_string(SUB_POW);
  }

  void attach_thread() {
    thread_id = thread_counter.fetch_add(1);
  }

  void detach_thread() {}

  bool find(Key key, Value &value) {
    value = container->get(key, thread_id);
    return value != 0;
  }

  bool insert(Key key, Value value) {
    bool res = container->putIfAbsent(key, value, thread_id);
    return res;
  }

  bool update(Key key, Value &value_expected, Value value_new) {
    container->putUpdate(key, value_expected, value_new, thread_id);
    return true;
  }

  bool remove(Key key) {
    int res = container->remove(key, thread_id);
    return res != 0;
  }

  size_t size() {
    return container->size();
  }

 private:
  WaitFreeHashTable<Key, Value>  *container;
};

#endif  // WF_HASHMAP_OLD_API_H_
