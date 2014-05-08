//
//  main_tester.c
//  mCAS
//
//  Created by Steven Feldman on 11/1/12.
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

#include "tervel/wf-ring-buffer/wf_ring_buffer.h"
#include "tervel/util/info.h"
#include "tervel/util/thread_context.h"
#include "tervel/util/tervel.h"
#include "tervel/util/memory/hp/hp_element.h"
#include "tervel/util/memory/hp/hp_list.h"

DEFINE_int32(num_threads, 8, "The number of threads to spawn.");
DEFINE_int32(execution_time, 1, "The amount of time to run the tests");
DEFINE_int32(buffer_length, 256, "The size of the region to test on.");
DEFINE_int32(operation_type, 0, "The type of test to execute"
    "(0: Both enqueue and dequeue"
    ", 1: Only Enqueue"
    ", 2: Only Dequeue)");

enum class TestType : size_t {ENQUEUEDEQUEUE = 0, ENQUEUE = 1, DEQUEUE = 2};

class TestObject {
 public:
  TestObject(int num_threads, int execution_time, int buffer_length,
             TestType test_type, RingBuffer *ring_buffer)
      : execution_time_(execution_time)
      , buffer_length_(buffer_length)
      , num_threads_(num_threads)
      , operation_type_(test_type)
      , rb_(*ring_buffer) {}

  void atomic_add(int enqueue_count, int dequeue_count) {
    enqueue_count_.fetch_add(enqueue_count);
    dequeue_count_.fetch_add(dequeue_count);
  }



  RingBuffer rb_;

  const int execution_time_;
  const int buffer_length_;
  const int num_threads_;
  const TestType operation_type_;

  std::atomic<uint64_t> enqueue_count_ {0};
  std::atomic<uint64_t> dequeue_count_ {0};
  std::atomic<void *>* shared_memory_;

  std::atomic<int> ready_count_ {0};
  std::atomic<bool> running_ {true};
  std::atomic<bool> wait_flag_ {true};
};


int buffer_length;

void run(int thread_id, tervel::Tervel* tervel_obj, TestObject * test_object);
void run_update_object(int start_pos, TestObject * test_data);

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  TestObject test_data(FLAGS_num_threads, FLAGS_execution_time,
        FLAGS_buffer_length, static_cast<TestType>(FLAGS_operation_type),
        new RingBuffer(FLAGS_buffer_length, FLAGS_num_threads));

  tervel::Tervel tervel_obj(test_data.num_threads_);
  tervel::ThreadContext tervel_thread(&tervel_obj);

  // run_update_object(0, &test_data);


  std::vector<std::thread> thread_list;
  for (int i = 0; i < test_data.num_threads_; i++) {
    std::thread temp_thread(run, i, &tervel_obj, &test_data);
    thread_list.push_back(std::move(temp_thread));
  }

  while (test_data.ready_count_.load() < test_data.num_threads_) {}

  test_data.wait_flag_.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(test_data.execution_time_));
  test_data.running_.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  printf("Signaled Stop!\n");

  std::for_each(thread_list.begin(), thread_list.end(), [](std::thread &t) {
      t.join();
  });

  printf("Completed[Passed: %lu, Failed: %lu]\n",
    test_data.enqueue_count_.load(), test_data.dequeue_count_.load());

  for (int i = 0; i < test_data.buffer_length_; i++) {
    printf("[%d: %p] ", i, test_data.shared_memory_[i].load());
  }

  return 1;
}


void run_enqueue_dequeue(int thread_id, TestObject * test_data);
void run_enqueue_only(int thread_id, TestObject * test_data);
void run_dequeue_only(int thread_id, TestObject * test_data);

void run(int thread_id, tervel::Tervel* tervel_obj, TestObject * test_data) {
  tervel::ThreadContext tervel_thread(tervel_obj);

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
}

void run_enqueue_dequeue(int thread_id, TestObject * test_data) {
  int enqueue_count = 0;
  int dequeue_count = 0;

  test_data->ready_count_.fetch_add(1);
  while (test_data->wait_flag_.load()) {}

  long val = (thread_id << 20) + 1; // give a unique id number and add one to
                                    // force an enqueue op to occur first
  int op_mask = 0x1;
  while (test_data->running_.load()) {
    val++;
    if (val & op_mask) {
      bool succ = test_data->rb.enqueue(val);
      if (succ) {
        enqueue_count++;
      }
    } else {
      long res = -1;
      bool succ = test_data->rb.dequeue(&res);
      if (succ) {
        assert(res != -1);
        dequeue_count++;
      }
    }
  }  // End Execution Loop

  test_data->atomic_add(enqueue_count, dequeue_count);
  return;
}

void run_enqueue_only(int start_pos, TestObject * test_data) {
  int enqueue_count = 0;

  test_data->ready_count_.fetch_add(1);
  while (test_data->wait_flag_.load()) {}

  long val = thread_id << 20;
  while (test_data->running_.load()) {
    val++;
    bool succ = test_data->rb.enqueue(val);
    if (succ) {
      enqueue_count++;
    }
  }  // End Execution Loop

  test_data->atomic_add(enqueue_count, 0);
  return;
}


void run_dequeue_only(int thread_id, TestObject * test_data) {
  int dequeue_count = 0;

  test_data->ready_count_.fetch_add(1);
  while (test_data->wait_flag_.load()) {}

  long val = thread_id << 20;
  while (test_data->running_.load()) {
    val++;
    long res = -1;
    bool succ = test_data->rb.dequeue(&res);
    if (succ) {
      assert(res != -1);
      dequeue_count++;
    }
  }  // End Execution Loop

  test_data->atomic_add(0, dequeue_count);
  return;
}
