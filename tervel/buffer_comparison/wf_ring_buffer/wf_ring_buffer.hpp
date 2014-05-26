#ifndef WF_RING_BUFFER_H
#define WF_RING_BUFFER_H

#include "wf_macros.hpp"
//#include "slowpath.hpp"

#include <limits.h>

#include <atomic>

const long kNotValue = LONG_MAX;
#include "node.h"

/*
struct Helper;

struct OpRec{
    Node *val;
    bool isEnq;
    bool isPending;
    std::atomic<Helper*> helper;
};

struct Helper{
    OpRec *op;
    unsigned long check_id;
};
*/

class WFRingBuffer {
 public:
  WFRingBuffer() { }

  void init(unsigned long num_threads, unsigned long capacity);
  void attachThread(){ threadID=activeThreads.fetch_add(1);}
  void dettachThread(){ activeThreads.fetch_add(-1); }
  Node* enqueue(unsigned long thread_id, Node *node);
  Node* dequeue(unsigned long thread_id);
  bool isFull();
  bool isEmpty();
  unsigned long numThreads() { return num_threads_; }
  unsigned int num_threads_;

 private:
  std::atomic<Node *> *queue_;
  //std::atomic<Node*> *queue_;
  std::atomic<void*> *opTable;
  //std::atomic<Helper*> *helpers_;
  
  unsigned long capacity_;
  unsigned long size_mask_;
  unsigned long back_off_;
  volatile unsigned long head_;
  volatile unsigned long tail_;

  int max_retry_;
  int max_fail_;
    
  unsigned long fetchHeadSeq();
  unsigned long fetchTailSeq();
  void tryHelpAnother(unsigned long thread_id);
  bool helpEnqueue(void *op);
  bool helpDequeue(void *op);
  void checkOpTable();
  void announceOp(void *op);
};

#endif  // WF_RING_BUFFER_H
