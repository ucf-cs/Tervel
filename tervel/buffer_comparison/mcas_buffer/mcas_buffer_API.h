#ifndef __mcas_buffer_h__
#define __mcas_buffer_h__

#include "WFRingBuffer.h"
#include "node.h"

struct Buffer {
 private:
  RingBuffer *buff;

 public:
  void q_init(unsigned long num_threads, unsigned long capacity) 
  {
      Init_RingBuffer_Memory_Management(num_threads); 

      buff = new RingBuffer(capacity); 

      #ifdef USE_MCAS_BUFFER
        INIT_THREAD_ID();
      #endif
  }
  bool q_enqueue(Node *node, long thread_id)
  {
      void *foo = (void *) node->val();
      return buff->enqueue(foo); 
  }
  bool q_dequeue(long thread_id) 
  { 
      void *foo = (void *) 0x1;
      return buff->dequeue(foo);
  }
  
  void q_finish() { }
  void q_init_thread() { }
  void q_finish_thread() { }
};
#endif
