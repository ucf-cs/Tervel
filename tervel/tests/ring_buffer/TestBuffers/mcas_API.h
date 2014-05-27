#ifndef __mcas_buffer_h__
#define __mcas_buffer_h__

#include "mcas/WFRingBuffer.h"

template<class T>
class TestBuffer {
 public:
  void TestBuffer(size_t capacity) {
    Init_RingBuffer_Memory_Management();
    INIT_THREAD_ID();
    queue_ = new RingBuffer(capacity);
  };

  char * name() {
    return "WF MCAS Buffer";
  }

  void attach_thread() {
    INIT_THREAD_ID();
  };

  void detach_thread() {
  };

  bool enqueue(T * val) {
    return queue_->enqueue(val);
  };
  bool dequeue() {
    T val = NULL;
    return queue_->dequeue(val);
  };

 private:
  RingBuffer *queue_;
};
#endif
