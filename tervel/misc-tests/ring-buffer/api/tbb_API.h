#ifndef TBB_QUEUE_H
#define TBB_QUEUE_H

#include "tbb/tbb.h"

template<class T>
class TestClass {
 public:
  TestClass(size_t capacity, size_t num_threads) {
    queue_ = new tbb::concurrent_bounded_queue<T>();
  };

  char * name() {
    return "TBB Bounded Queue";
  }

  void attach_thread() {};

  void detach_thread() {};

  bool enqueue(T val) {
    return queue_->try_push(val);
  };
  bool dequeue(T &val) {
    return queue_->try_pop(val);
  };

 private:
  tbb::concurrent_bounded_queue<T> *queue_;
};



#endif  // TBB_QUEUE_H
