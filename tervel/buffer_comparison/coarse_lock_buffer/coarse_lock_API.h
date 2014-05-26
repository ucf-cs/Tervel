#ifndef __coarse_lock_buffer_h__
#define __coarse_lock_buffer_h__

#include "coarse_lock_buffer.h"
#include "node.h"

struct Buffer {
 private:
  CoarseLockBuffer buff;

 public:
  void q_init(unsigned long num_threads, unsigned long capacity) { buff.init(num_threads, capacity); }
  bool q_enqueue(Node *node, long thread_id) { return buff.enqueue(node); }
  bool q_dequeue(long thread_id) { return buff.dequeue(); }

  void q_finish() { }
  void q_init_thread() { }
  void q_finish_thread() { }
};
#endif
