#ifndef ONLY_ATOMIC_FAA_H
#define ONLY_ATOMIC_FAA_H

#include <atomic>

template<class T>
class TestBuffer {
 public:
    TestBuffer(size_t capacity, size_t num_threads))
     : head_(0)
     , tail_(0) { }

    char * name() {
      return "FAA Test";
    }
    bool enqueue(T value) {
        fetchTailSeq();
        return true;
    };

    bool dequeue(T &val) {
        fetchHeadSeq();
        return true;
    };

    void attach_thread() {
      return;
    };

    void detach_thread() {
      return;
    };

 private:
    uint64_t fetchHeadSeq() {
        return head.fetch_add(1);
    };

    uint64_t fetchTailSeq() {
        return tail.fetch_add(1);
    };

    std::atomic<uint64_t> head_;
    std::atomic<uint64_t> tail_;
};

#endif  // WF_RING_BUFFER_H
