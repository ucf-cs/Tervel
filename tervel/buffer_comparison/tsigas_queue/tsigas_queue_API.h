#ifndef __TSIGAS_QUEUE_API_H__
#define __TSIGAS_QUEUE_API_H__

#include "tsigas_queue.h"
#include "node.h"

struct Buffer {
 private:
  TsigasQueue buff;

 public:
  void q_init(unsigned long num_threads, unsigned long capacity) { buff.init(num_threads, capacity); }
  void q_finish() { buff.finish(); }
  void q_init_thread() { buff.attach_thread(); }
  void q_finish_thread() { buff.detach_thread(); }
  bool q_enqueue(Node *node, long thread_id) { return buff.enqueue(node); }
  bool q_dequeue(long thread_id) { return buff.dequeue(); }
};
#endif
