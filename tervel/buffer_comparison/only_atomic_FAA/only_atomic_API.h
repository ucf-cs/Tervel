#ifndef __only_atomic_FAA_h__
#define __only_atomic_FAA_h__

#include "only_atomic_FAA.h"
#include "node.h"

struct Buffer {
 private:
  OnlyAtomicFAA buff;

 public:
  void q_init(unsigned long num_threads, unsigned long capacity) { }
  bool q_enqueue(Node *node, long thread_id) { return buff.enqueue(); }
  bool q_dequeue(long thread_id) { return buff.dequeue(); }

  void q_finish() { }
  void q_init_thread() { }
  void q_finish_thread() { }
};
#endif
