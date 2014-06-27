#ifndef WF_HASHMAP_API_H_
#define WF_HASHMAP_API_H_

#include "tervel/containers/wf/hash-map/wf_hash_map.h"
#include "tervel/util/info.h"
#include "tervel/util/thread_context.h"
#include "tervel/util/tervel.h"
#include "tervel/util/memory/hp/hp_element.h"
#include "tervel/util/memory/hp/hp_list.h"

template<class Key, class Value>
class TestClass {
 public:
  TestClass(size_t num_threads, size_t capacity) {
    tervel_obj = new tervel::Tervel(num_threads);
    attach_thread();
    container = new tervel::containers::wf::HashMap<Key, Value>(capacity);
  }

  char * name() {
    return "WF Hash Map";
  }

  void attach_thread() {
    tervel::ThreadContext* thread_context __attribute__((unused));
    thread_context = new tervel::ThreadContext(tervel_obj);
  }

  void detach_thread() {}

  bool find(Key key, Value &value) {
    return container->find(key, value);
  }

  bool insert(Key key, Value &value) {
    return container->insert(key, value);
  }

  bool update(Key key, Value &value_expected, Value value_new) {
    return container->update(key, value_expected, value_new);
  }

  bool remove(Key key, Value &value_expected) {
    return container->remove(key, value_expected);
  }

  size_t size() {
    return container->size();
  }

 private:
  tervel::Tervel* tervel_obj;
  tervel::containers::wf::HashMap<Key, Value> *container;
};

#endif  // WF_HASHMAP_API_H_
