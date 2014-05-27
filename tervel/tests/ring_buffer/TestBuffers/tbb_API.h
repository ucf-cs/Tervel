#ifndef TBB_QUEUE_H
#define TBB_QUEUE_H

#include "tbb/tbb.h"

template<class T>
class TestBuffer {
 public:
  void TestBuffer(size_t capacity) {
    queue_ = new tbb::concurrent_bounded_queue<T>(capacity);
  };

  char * name() {
    return "TBB Bounded Queue";
  }

  void attach_thread() {};

  void detach_thread() {};

  bool enqueue(T * val) {
    return queue_->try_push(val);
  };
  bool dequeue() {
    T val = NULL;
    return queue_->try_pop(val);
  };

 private:
  tbb::concurrent_bounded_queue<T> *queue_;
};



#endif  // TBB_QUEUE_H
