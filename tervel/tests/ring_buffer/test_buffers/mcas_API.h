#ifndef __mcas_buffer_h__
#define __mcas_buffer_h__

#include "tervel/util/info.h"
#include "tervel/util/thread_context.h"
#include "tervel/util/tervel.h"

#include "tervel/containers/lf/mcas-buffer/mcas_buffer.h"

template<class T>
class TestBuffer {
 public:
  TestBuffer(size_t capacity, size_t num_threads) {
    tervel_obj = new tervel::Tervel(num_threads);
    attach_thread();
    queue_ = new tervel::containers::lf::mcas_buffer::RingBuffer<T>(capacity);
  };

  char * name() {
    return "WF MCAS Buffer";
  }

  void attach_thread() {
    tervel::ThreadContext* thread_context __attribute__((unused));
    thread_context = new tervel::ThreadContext(tervel_obj);
  };

  void detach_thread() {
  };

  bool enqueue(T val) {
    return queue_->enqueue(val);
  };

  bool dequeue(T &val) {
    return queue_->dequeue(val);
  };

  void print_queue() {
    queue_->print_queue();
  }

 private:
  tervel::Tervel *tervel_obj;
  tervel::containers::lf::mcas_buffer::RingBuffer<T> *queue_;
};

#endif  // __mcas_buffer_h__
