#ifndef API_CDS_SPLIT_MAP_H_
#define API_CDS_SPLIT_MAP_H_
#define USE_CDS 1

#include <cds/init.h>
#include <cds/gc/hp.h>
#include <cds/opt/hash.h>
#include <cds/container/michael_kvlist_hp.h>
#include <cds/container/michael_map.h>

template<class Key, class Value>
class TestClass {
  template <class T>
  struct less_s : std::binary_function <T,T,bool> {
    bool operator() (const T& x, const T& y) const {
      return (memcmp(&x, &y, sizeof(T)) < 0);
    }
  };


  // List traits based on std::less predicate
  struct list_traits: public cds::container::michael_list::type_traits {
    typedef struct less_s<Key> less;
  };

  // Ordered list
  typedef typename cds::container::MichaelKVList< cds::gc::HP, Key, Value, list_traits> Key2Value_list;
  // Map traits
  struct map_traits: public cds::container::michael_map::type_traits {
    struct hash {
      size_t operator()(const Key &k) const {
        size_t hash_v = 0;
        char *temp = (char *)(&k);
        for (int i = 0; i < sizeof(Key); i++) {
          hash_v += temp[i];
        }
        hash_v = hash_v + 1;
        return hash_v;
      }
    };
  };

  typedef typename cds::container::MichaelHashMap<cds::gc::HP, Key2Value_list,
    map_traits > hash_t ;

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
    test_container = new hash_t(capacity, test_michael_map);
  }

  char * name() {
    const int t = test_michael_map;
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
