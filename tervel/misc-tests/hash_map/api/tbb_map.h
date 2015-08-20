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

#ifndef TBB_MAP_API_H_
#define TBB_MAP_API_H_


#include <tbb/concurrent_hash_map.h>
using namespace tbb;


template<class Key, class Value>
class TestClass {
 private:
  struct MyHashCompare {
    static size_t hash(const Key &k) {
      size_t hash_v = 0;
      char * temp = (char *)(&k);
      for (int i = 0; i < sizeof(Key); i++) {
        hash_v += temp[i];
      }
      hash_v = hash_v + 1;
      return hash_v;
    }

    static bool equal(const Key &key1, const Key &key2) {
      bool res = memcmp(&key1, &key2, sizeof(Key)) == 0;
      return res;
    }
  };
  typename tbb::concurrent_hash_map<Key, Value, MyHashCompare> * container;

 public:
  TestClass(size_t num_threads, size_t capacity) {
    container = new tbb::concurrent_hash_map<Key, Value, MyHashCompare>
      (capacity);
  }

  std::string toString() {
    return "TBB Map";
  }

  void attach_thread() {}
  void detach_thread() {}

  bool find(Key key, Value &value) {
    typename tbb::concurrent_hash_map<Key, Value, MyHashCompare>::const_accessor a;

    if (container->find(a, key)) {
       value = a->second;
       return true;
    } else {
      return false;
    }
  };

  bool insert(Key key, Value value) {
    return container->insert( std::make_pair(key, value) );
  };

  bool update(Key key, Value &value_expected, Value value_new) {
    typename tbb::concurrent_hash_map<Key, Value, MyHashCompare>::accessor a;

    if (container->find(a, key)) {
      if (a->second == value_expected) {
        a->second = value_new;
        return true;
      } else {
        value_expected = a->second;
        return false;
      }
    }
    return false;
  };

  bool remove(Key key) {
    return container->erase(key);
  };

  size_t size() {
    return container->size();
  };
};

#endif  //
