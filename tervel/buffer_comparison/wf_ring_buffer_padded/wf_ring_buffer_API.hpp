#ifndef __coarse_lock_buffer_h__
#define __coarse_lock_buffer_h__

#include "wf_ring_buffer.hpp"
#include "node.h"

struct Buffer {
 private:
  WFRingBuffer buff;

 public:
  void q_init(unsigned long num_threads, unsigned long capacity) { buff.init(num_threads, capacity); }
  Node* q_enqueue(Node *node, long thread_id) { return buff.enqueue(thread_id, node); }
  Node* q_dequeue(long thread_id) { return buff.dequeue(thread_id); }
  
  void q_finish() { }
  void q_init_thread() { buff.attachThread(); }
  void q_finish_thread() { buff.dettachThread(); }
};
#endif
