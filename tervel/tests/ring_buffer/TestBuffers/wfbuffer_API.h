#ifndef WFBUFFER_API_H_
#define WFBUFFER_API_H_

#include "tervel/wf-ring-buffer/wf_ring_buffer.h"
#include "tervel/util/info.h"
#include "tervel/util/thread_context.h"
#include "tervel/util/tervel.h"
#include "tervel/util/memory/hp/hp_element.h"
#include "tervel/util/memory/hp/hp_list.h"

template<class T>
class TestBuffer {
 public:
  void TestBuffer(size_t capacity) {
    queue_ = new tervel::RingBuffer<T>(capacity);
    tervel_obj = new tervel::Tervel(FLAGS_num_threads);
  }

  char * name() {
    return "WF Ring Buffer";
  }

  void attach_thread() {
    tervel::ThreadContext* thread_context =
      new tervel::ThreadContext(tervel_obj);
  }

  void detach_thread() {}

  bool enqueue(T * val) {
    return queue_->enqueue(val);
  }

  bool dequeue() {
    T val = NULL;
    return queue_->dequeue(val);
  }

 private:
  tervel::Tervel* tervel_obj,
  tervel::RingBuffer<T> *queue_;
};



#endif  // WFBUFFER_API_H_
