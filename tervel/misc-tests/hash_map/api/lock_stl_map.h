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

#ifndef API_LOCK_STL_H_
#define API_LOCK_STL_H_

#include <boost/thread.hpp>
#include <unordered_map>
#include <tr1/unordered_map>

template<class Key, class Value>
class TestClass {

  template<class T>
  struct s_hash : public std::unary_function<T, std::size_t> {
    std::size_t operator()(T const &k) const{
      size_t hash_v = 0;
      char * temp = (char *)(&k);
      for (int i = 0; i < sizeof(Key); i++) {
        hash_v += temp[i];
      }
      hash_v = hash_v + 1;
      return hash_v;
    }
  };

  typedef struct s_hash<Key> c_hash;

  template <class T> struct equal_to : std::binary_function <T,T,bool> {
    bool operator() (const T& x, const T& y) const
    {
      bool res = memcmp(&x, &y, sizeof(Key)) == 0;
      return res;
    }
  };
  typedef struct equal_to<Key> c_equals;

  typedef typename std::tr1::unordered_map<Key, Value, c_hash, c_equals,  std::allocator<std::pair<Key, Value> > >  hash_t;
 public:
  TestClass(size_t num_threads, size_t capacity) {
    test_container = new hash_t(capacity);

  }

  std::string toString() {
    return "Locked STL Map";
  }

  void attach_thread() {}

  void detach_thread() {}

  bool find(Key key, Value &value) {
    boost::mutex::scoped_lock lock(v_mutex);
    bool res = false;
    typename hash_t::iterator iter = test_container->find(key);
    if(iter != test_container->end()) {
      value = iter->second;
      res = true;
    }

    return res;
  }

  bool insert(Key key, Value value) {
    boost::mutex::scoped_lock lock(v_mutex);
    std::pair<typename hash_t::iterator, bool> res = test_container->insert(std::make_pair(key,value));

    return res.second;
  }

  bool update(Key key, Value &value_expected, Value value_new) {
    boost::mutex::scoped_lock lock(v_mutex);
    bool res = false;
    typename hash_t::iterator iter = test_container->find(key);
    if(iter != test_container->end()) {
      if (iter->second == value_expected) {
        iter->second = value_new;
        res = true;
      } else {
        value_expected = iter->second;
      }

    }

    return res;
  }

  bool remove(Key key) {
    boost::mutex::scoped_lock lock(v_mutex);
    int res= test_container->erase(key);

    return res;
  }

  size_t size() {
    return test_container->size();
  }

 private:
  boost::mutex v_mutex;
  hash_t *test_container;

};

#endif  //API_LOCK_STL_H_
