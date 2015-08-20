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

#ifndef API_CDS_SPLIT_MAP_H_
#define API_CDS_SPLIT_MAP_H_
#include <cds/init.h>
#include <cds/gc/hp.h>
#include <cds/opt/hash.h>
#include <cds/container/michael_list_hp.h>
#include <cds/container/split_list_map.h>

#define USE_CDS

template<class Key, class Value>
class TestClass {
  template<class T>
  struct s_hash : public std::unary_function<T, std::size_t> {
    std::size_t operator()(T const &k) const{
      size_t hash_v = 0;
      char * temp = (char *)(&k);
      for (int i = 0; i < sizeof(T); i++) {
        hash_v += temp[i];
      }
      hash_v = hash_v + 1;
      return hash_v;
    }
  };

  template <class T> struct less_s : std::binary_function <T,T,bool> {
    bool operator() (const T& x, const T& y) const
    {
      return (memcmp(&x, &y, sizeof(T)) < 0);
    }
  };


  // SplitListMap traits
  struct foo_set_traits: public cds::container::split_list::type_traits
  {
    typedef typename cds::container::michael_list_tag ordered_list;
    typedef struct s_hash<Key> hash;
    // Type traits for our MichaelList class
    struct ordered_list_traits: public cds::container::michael_list::type_traits
    {
      typedef struct less_s<Key> less;   // use our std::less predicate as comparator to order list nodes
    } ;
  };

  typedef typename cds::container::SplitListMap< cds::gc::HP, Key, Value, foo_set_traits> hash_t ;

  struct functor
  {
    Value curr_value;
    bool res;
    void operator ()(std::pair<Key, Value> pair) {
      curr_value = pair.second;
      res = true;
    }

    bool getValue(Value &val) {
      if (res) {
        val = curr_value;
      }
      return res;
    }
  };

  struct ufunctor {
    Value value_new;
    Value value_expected;
    bool res;

    void operator ()( bool bNew, std::pair<Key const, Value >& item) {
      if (bNew) {
        res = false;
      } else {
        std::atomic<Value> * address = (std::atomic<Value> *)&(item.second);
        res = address->compare_exchange_strong(value_expected, value_new);
      }
    }

    bool getValue(Value &val) {
      if (!res) {
        val = value_expected;
      }
      return res;
    }
  };

 public:
  TestClass(size_t num_threads, size_t capacity) {
    test_container = new hash_t(capacity, test_splitlist);
  }

  std::string toString() {
    const int t = test_splitlist;
    return "CDS Split Map(" + std::to_string(t) + ")";
  }

  void attach_thread() {
  }

  void detach_thread() {}

  bool find(Key key, Value &value) {
    struct functor ef;
    if (test_container->find(key, cds::ref(ef))) {
      return ef.getValue(value);
    } else {
      return false;
    }
  }

  bool insert(Key key, Value value) {
    return test_container->insert(key, value);
  }

  bool update(Key key, Value &value_expected, Value value_new) {
    struct ufunctor uf;
    uf.value_expected = value_expected;
    uf.value_new = value_new;
    uf.res = false;

    test_container->ensure(key, cds::ref(uf));
    return uf.getValue(value_expected);
  }

  bool remove(Key key) {
    return test_container->erase(key);
  }

  size_t size() {
    return test_container->size();
  }

 private:
  hash_t *test_container;
};

#endif  //API_CDS_SPLIT_MAP_H_
