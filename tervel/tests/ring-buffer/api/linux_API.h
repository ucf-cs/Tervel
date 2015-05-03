#ifndef LINUX_API_H_
#define LINUX_API_H_

#include "linux_buffer/lockfree_rb_q.cc"

template<class T>
class TestClass {
 public:
    TestClass(size_t capacity, size_t num_threads)
     : buff_(new LockFreeQueue<T>(num_threads, num_threads, capacity)) {}

    char * name() {
      return "Linux Buffer";
    }
    bool enqueue(T value) {
        buff_->push(value);
        return true;
    };

    bool dequeue(T &val) {
        val = buff_->pop();
        return true;
    };

    void attach_thread() {
      return;
    };

    void detach_thread() {
      return;
    };

 private:
    LockFreeQueue<T> *buff_;
};

#endif  // LINUX_API_H_
