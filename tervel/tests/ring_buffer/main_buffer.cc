//
//  main_tester.c
//  mCAS
//
//  Created by Steven Feldman on 11/1/12.
//  Perfected by Andrew Barrington :p
//  Copyright (c) 2012 Steven FELDMAN. All rights reserved.
//

// #define KILL_THREAD 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/time.h>
#include <time.h>
#include <vector>
#include <thread>

#include <iostream>
#include <atomic>

#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>
#include <gflags/gflags.h>

#include "bufferAPI.h"

DEFINE_int32(prefill, 50, "The prefill percent 0-100 (default 50).");
DEFINE_int32(num_threads, 8, "The number of threads to spawn.");
DEFINE_int32(execution_time, 3, "The amount of time to run the tests");
DEFINE_int64(buffer_length, 65536, "The size of the region to test on.");
DEFINE_int32(operation_type, 0, "The type of test to execute"
    "(0: Both enqueue and dequeue"
    ", 1: Only Enqueue"
    ", 2: Only Dequeue)");
DEFINE_int64(enqueue_rate, 50, "The percent of enqueue operations (0-100).");

enum class TestType : size_t {ENQUEUEDEQUEUE = 0, ENQUEUE = 1, DEQUEUE = 2};

class TestObject {
 public:
  TestObject(int num_threads, int prefill, int execution_time,
            int buffer_length, TestType test_type, int enqueue_rate)
      : enqueue_rate_(enqueue_rate)
      , execution_time_(execution_time)
      , buffer_length_(buffer_length)
      , num_threads_(num_threads)
      , prefill_(prefill)
      , operation_type_(test_type)
      , rb_(buffer_length, num_threads) {}

  void atomic_add(int enqueue_count, int dequeue_count) {
    enqueue_count_.fetch_add(enqueue_count);
    dequeue_count_.fetch_add(dequeue_count);
  }

  void print_test() {
    printf("Execution Time: %d\n", execution_time_);
    printf("Num Threads: %d\n", num_threads_);
    printf("Buffer Length: %ld\n", buffer_length_);
    printf("Enqueue Rate: %d\n", enqueue_rate_);
    printf("Prefill: %d\n", prefill_);
    printf("Operation Type: %d\n", static_cast<int>(operation_type_));
    printf("Buffer Type: %s\n", rb_.name() );
  };


  const int enqueue_rate_;
  const int execution_time_;
  const int64_t buffer_length_;
  const int num_threads_;
  const int prefill_;
  const TestType operation_type_;

  std::atomic<uint64_t> enqueue_count_{0};
  std::atomic<uint64_t> dequeue_count_{0};
  std::atomic<void *>* shared_memory_;

  std::atomic<int> ready_count_{0};
  std::atomic<bool> running_{true};
  std::atomic<bool> wait_flag_{true};

  TestBuffer<int64_t> rb_;
};


void run(int thread_id, TestObject * test_object);
void run_update_object(int start_pos, TestObject * test_data);

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  TestObject test_data(FLAGS_num_threads+1, FLAGS_prefill, FLAGS_execution_time,
        FLAGS_buffer_length, static_cast<TestType>(FLAGS_operation_type),
        FLAGS_enqueue_rate);
#ifdef DEBUG
  test_data.print_test();
#endif

#ifdef USING_CDS_LIB
  // Initialize libcds
  cds::Initialize() ;
  {
    // Initialize Hazard Pointer singleton
    cds::gc::HP hpGC ;
    // If main thread uses lock-free containers
    // the main thread should be attached to libcds infrastructure
    cds::threading::Manager::attachThread() ;
#endif

  // prefill the buffer if needed
  if (test_data.operation_type_ == TestType::ENQUEUEDEQUEUE) {
    const int64_t items = test_data.buffer_length_ * (test_data.prefill_/100.0);
    for (int64_t i = 1; i < items ; i++) {
      test_data.rb_.enqueue(i);
    }
  }

  std::vector<std::thread> thread_list;
  for (int64_t i = 0; i < test_data.num_threads_; i++) {
    std::thread temp_thread(run, i, &test_data);
    thread_list.push_back(std::move(temp_thread));
  }

  while (test_data.ready_count_.load() < test_data.num_threads_) {}

#ifdef DEBUG
  printf("Beginning Test.\n");
#endif
  test_data.wait_flag_.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(test_data.execution_time_));
  test_data.running_.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(1));
#ifdef DEBUG
  printf("Signaled Stop!\n");
#endif

  std::for_each(thread_list.begin(), thread_list.end(), [](std::thread &t) {
      t.join();
  });

  #if DEBUG
  rb->print_buffer_contents()
  rb->print_lost_nodes();
  #endif  // DEBUG

#ifdef DEBUG
  printf("Completed[Enqueues: %lu, Dequeues: %lu]\n",
    test_data.enqueue_count_.load(), test_data.dequeue_count_.load());
#else
  printf("%lu\t%lu", test_data.enqueue_count_.load(),
          test_data.dequeue_count_.load());
#endif

#ifdef USING_CDS_LIB
  }
#endif
  return 0;
}


void run_enqueue_dequeue(int thread_id, TestObject * test_data);
void run_enqueue_only(int thread_id, TestObject * test_data);
void run_dequeue_only(int thread_id, TestObject * test_data);

void run(int thread_id, TestObject * test_data) {
  test_data->rb_.attach_thread();

  switch (test_data->operation_type_) {
    case TestType::ENQUEUEDEQUEUE:
      run_enqueue_dequeue(thread_id, test_data);
      break;

    case TestType::ENQUEUE:
      run_enqueue_only(thread_id, test_data);
      break;

    case TestType::DEQUEUE:
      run_dequeue_only(thread_id, test_data);
      break;

    default:
      printf("Error Non Recongized State Test\n");
  }

  test_data->rb_.detach_thread();
}

void run_enqueue_dequeue(int thread_id, TestObject * test_data) {
  int enqueue_count = 0;
  int dequeue_count = 0;
  int64_t val = (thread_id << 24) + 1;

  boost::mt19937 rng((int64_t)thread_id);
  boost::uniform_int<> opRand(1, 100);

  test_data->ready_count_.fetch_add(1);
  while (test_data->wait_flag_.load()) {}

  while (test_data->running_.load()) {
    int op = opRand(rng);

    if (op <= test_data->enqueue_rate_) {
      bool succ = test_data->rb_.enqueue(val++);
      if (succ) {
        enqueue_count++;
      }
    } else {
      int64_t res = 0;
      bool succ = test_data->rb_.dequeue(res);
      if (succ) {
        assert(res != 0);
        dequeue_count++;
      }
    }
  }  // End Execution Loop

  test_data->atomic_add(enqueue_count, dequeue_count);
}

void run_enqueue_only(int thread_id, TestObject * test_data) {
  int enqueue_count = 0;

  test_data->ready_count_.fetch_add(1);
  while (test_data->wait_flag_.load()) {}

  int64_t val = thread_id << 20;
  while (test_data->running_.load()) {
    val++;
    bool succ = test_data->rb_.enqueue(val);
    if (succ) {
      enqueue_count++;
    }
  }  // End Execution Loop

  test_data->atomic_add(enqueue_count, 0);
}


void run_dequeue_only(int thread_id, TestObject * test_data) {
  int dequeue_count = 0;

  test_data->ready_count_.fetch_add(1);
  while (test_data->wait_flag_.load()) {}

  int64_t val = thread_id << 20;
  while (test_data->running_.load()) {
    val++;
    int64_t res = 0;
    bool succ = test_data->rb_.dequeue(res);
    if (succ) {
      assert(res != 0);
      dequeue_count++;
    }
  }  // End Execution Loop

  test_data->atomic_add(0, dequeue_count);
}
