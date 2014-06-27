#ifndef WF_HASHMAP_API_H_
#define WF_HASHMAP_API_H_

#include "tervel/hashmap/wf_hash_map.h"
#include "tervel/util/info.h"
#include "tervel/util/thread_context.h"
#include "tervel/util/tervel.h"
#include "tervel/util/memory/hp/hp_element.h"
#include "tervel/util/memory/hp/hp_list.h"

template<class T>
class TestClass {
 public:
  TestClass(size_t num_threads, size_t capacity) {
    tervel_obj = new tervel::Tervel(num_threads);
    attach_thread();
    container = new tervel::containers::wf::HashMap<T, T>(capacity);
  }

  char * name() {
    return "WF Hash Map";
  }

  void attach_thread() {
    tervel::ThreadContext* thread_context __attribute__((unused));
    thread_context = new tervel::ThreadContext(tervel_obj);
  }

  void detach_thread() {}


 private:
  tervel::Tervel* tervel_obj;
  tervel::containers::wf::HashMap<uint64_t, uint64_t> *container;
};

#endif  // WF_HASHMAP_API_H_
