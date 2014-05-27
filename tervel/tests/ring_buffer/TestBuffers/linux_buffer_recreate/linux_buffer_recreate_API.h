#ifndef __LINUX_BUFFER_RECREATE_H__
#define __LINUX_BUFFER_RECREATE_H__

#include "linux_buffer_recreate.h"
#include "node.h"

struct Buffer {
 private:
  LinuxBuffer buff;

 public:
  void q_init(unsigned long num_threads, unsigned long capacity) { buff.init(num_threads, capacity); }
  Node* q_enqueue(Node *node, long thread_id) { return buff.enqueue(node, thread_id); }
  Node* q_dequeue(long thread_id) { return buff.dequeue(thread_id); }

  void q_finish() { }
  void q_init_thread() { }
  void q_finish_thread() { }
};
#endif // __LINUX_BUFFER_RECREATE_H__

