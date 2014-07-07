#ifndef WF_VECTOR_API_H_
#define WF_VECTOR_API_H_

#include "tervel/containers/wf/vector/vector.hpp"
#include "tervel/util/info.h"
#include "tervel/util/thread_context.h"
#include "tervel/util/tervel.h"
#include "tervel/util/memory/hp/hp_element.h"
#include "tervel/util/memory/hp/hp_list.h"

template<typename T>
class TestClass {
 public:
  TestClass(size_t num_threads, size_t capacity) {
    tervel_obj = new tervel::Tervel(num_threads);
    attach_thread();
    container = new tervel::containers::wf::vector::Vector<T>(capacity);
  }

  char * name() {
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

  bool pop_back(const T &value) {
    assert(false);
    return false;
  };

  size_t size() {
    return container->size();
  };

 private:
  tervel::Tervel* tervel_obj;
  tervel::containers::wf::vector::Vector<T> *container;
};

#endif  // WF_VECTOR_API_H_
