#ifndef COARSE_LOCK_BUFFER_H
#define COARSE_LOCK_BUFFER_H

#include <atomic>

template<class T>
class TestBuffer {
 public:
  TestBuffer(size_t capacity, size_t num_threads))
    : capacity_(capacity)
    , size_mask_(capacity_)
    , head_(0)
    , tail_(0)
    , queue_ = new std::atomic<T>[capacity_] {
    pthread_mutex_init(&queue_lock_, NULL);
    for (size_t i = 0; i < capacity_; i++) {
        queue_[i].store(nullptr);
    }
  };

  char * name() {
    return "Coarse Lock";
  }

  void attach_thread() {}

  void detach_thread() {}

  bool enqueue(T val) {
    pthread_mutex_lock(&queue_lock_);
    bool res = false;
    if (!isFull()) {
      queue_[fetchHeadSeq() & size_mask_] = node;
    }

    pthread_mutex_unlock(&queue_lock_);
    return res;
  }

  bool dequeue(T &val) {
    pthread_mutex_lock(&queue_lock_);
    bool res = false;
    if (!isEmpty()) {
      val = queue_[fetchTailSeq() & size_mask_];
      res = true;
    }

    pthread_mutex_unlock(&queue_lock_);
  }

 private:
  uint64_t fetchHeadSeq() {
    return head_++;
  }

  uint64_t fetchTailSeq() {
    return tail_++;
  }

  bool isFull() {
    return (tail_ == head_+capacity_);
  }

  bool isEmpty() {
    return (head_ == tail_);
  }

  uint64_t capacity_;
  uint64_t size_mask_;
  uint64_t head_;
  uint64_t tail_;

  std::atomic<T> *queue_;

  pthread_mutex_t queue_lock_;
};

#endif  // COARSE_LOCK_BUFFER_H
