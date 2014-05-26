#ifndef __tbb_queue_h__
#define __tbb_queue_h__

#include "tbb_queue.h"
#include "node.h"

struct Buffer {
 private:
  TbbQueue buff;

 public:
  void q_init(unsigned long num_threads, unsigned long capacity) { buff.init(num_threads, capacity); }
  bool q_enqueue(Node *node, long thread_id) { return buff.enqueue(node); }
  bool q_dequeue(long thread_id) { return buff.dequeue(); }
  
  void q_finish() { }
  void q_init_thread() { }
  void q_finish_thread() { }
};
#endif
