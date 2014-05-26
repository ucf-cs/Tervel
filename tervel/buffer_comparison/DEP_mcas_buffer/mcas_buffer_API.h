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
  }
  Node* q_enqueue(Node *node, long thread_id)
  {
      void *foo = (void *) node->val();
      buff->enqueue(foo); 
      return node;
  }
  Node* q_dequeue(long thread_id) 
  { 
      void *foo = (void *) 0x1;
      buff->dequeue(foo);
      return NULL;
  }
};
#endif
