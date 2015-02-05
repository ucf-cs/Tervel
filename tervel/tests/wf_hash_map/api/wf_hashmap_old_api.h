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
