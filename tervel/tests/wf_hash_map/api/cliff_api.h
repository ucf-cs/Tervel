#ifndef BLANK_MAP_API_H
#define BLANK_MAP_API_H

 extern "C" {
#include "/Libraries/nbds.0.4.3/include/common.h"
#include "/Libraries/nbds.0.4.3/include/nstring.h"
#include "/Libraries/nbds.0.4.3/include/runtime.h"
#include "/Libraries/nbds.0.4.3/include/map.h"
#include "/Libraries/nbds.0.4.3/include/rcu.h"
#include "/Libraries/nbds.0.4.3/include/list.h"
#include "/Libraries/nbds.0.4.3/include/skiplist.h"
#include "/Libraries/nbds.0.4.3/include/hashtable.h"
 }

template<class Key, class Value>
class TestClass {
 public:
  TestClass(size_t num_threads, size_t capacity) {
    container = map_alloc(&MAP_IMPL_HT, NULL);
  }

  std::string name() {
    return "Cliff Click";
  }

  void attach_thread() {}

  void detach_thread() {}

  bool find(Key key, Value &value) {
    map_key_t temp = (map_key_t) key;

    map_val_t temp2 = map_get(container, temp);
    if (temp2 != 0) {
      value = temp2;
      return true;
    } else {
      return false;
    }
  }

  bool insert(Key key, Value value) {
    map_val_t temp = map_add(container, (map_key_t) key,  (map_val_t) value);
    if (temp == 0) {
      return true;
    } else {
      return false;
    }
  }

  bool update(Key key, Value &value_expected, Value value_new) {
    map_val_t temp = map_cas(container, (map_key_t) key,
      (map_val_t) value_expected, (map_val_t) value_new);
    if (temp == value_expected) {
      return true;
    } else {
      value_expected = false;
      return false;
    }
  }

  bool remove(Key key) {
    map_val_t temp = map_remove(container, (map_key_t) key);
    if (temp != 0) {
      return true;
    } else {
      return false;
    }
  }

  size_t size() {
    return -1;
  }

 private:
  map_t *container;
};

#endif  //
