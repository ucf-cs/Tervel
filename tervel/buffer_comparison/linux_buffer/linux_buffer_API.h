#ifndef __LINUX_BUFFER_H__
#define __LINUX_BUFFER_H__

#include "lockfree_rb_q.cc"
#include "node.h"

struct Buffer {
 private:
  LockFreeQueue<Node> buff;

 public:
  void q_init(unsigned long num_threads, unsigned long capacity) { buff.init(num_threads, capacity); }
  Node* q_enqueue(Node *node, long thread_id) { buff.push(node); return node; }
  Node* q_dequeue(long thread_id) { return buff.pop(); }

  void q_finish() { }
  void q_init_thread() { }
  void q_finish_thread() { }
};
#endif // __LINUX_BUFFER_H__
