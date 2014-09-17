#ifndef BLANK_MAP_API_H
#define BLANK_MAP_API_H

template<class Key, class Value>
class TestClass {
 public:
  TestClass(size_t num_threads, size_t capacity) {
  }

  char * name() {
    return "Blank Map";
  }

  void attach_thread() {
  }

  void detach_thread() {}

  bool find(Key key, Value &value) {
  }

  bool insert(Key key, Value value) {
  }

  bool update(Key key, Value &value_expected, Value value_new) {
   }

  bool remove(Key key) {
  }

  size_t size() {
    return container->size();
  }

 private:
};

#endif  //
