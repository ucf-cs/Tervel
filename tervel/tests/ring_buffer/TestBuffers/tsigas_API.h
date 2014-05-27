#ifndef __TSIGAS_BUFFER_H__
#define __TSIGAS_BUFFER_H__

#include <cds/container/tsigas_cycle_queue.h>
#define USING_CDS_LIB 1


template<class T>
class TestBuffer {
 public:
  TestBuffer(size_t capacity, size_t num_threads) {
    queue_ = new TQueue(capacity);
  };

  char * name() {
    return "CDS Tsiagas Cycle Queue";
  }

  void attach_thread() {
    cds::threading::Manager::attachThread();
  };

  void detach_thread() {
    cds::threading::Manager::detachThread();
  };

  bool enqueue(T val) {
    return queue_->enqueue(val);
  };
  bool dequeue(T &val) {
    return queue_->dequeue(val);
  };

 private:
  typedef cds::container::TsigasCycleQueue<T,
      cds::opt::buffer< cds::opt::v::dynamic_buffer<T> >  > TQueue;
  TQueue *queue_;
};

#endif  // __TSIGAS_BUFFER_H__
