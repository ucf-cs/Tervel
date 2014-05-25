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

#include "tervel/wf-ring-buffer/wf_ring_buffer.h"
#include "tervel/util/info.h"
#include "tervel/util/thread_context.h"
#include "tervel/util/tervel.h"
#include "tervel/util/memory/hp/hp_element.h"
#include "tervel/util/memory/hp/hp_list.h"

using namespace tervel::wf_ring_buffer;

DEFINE_int32(num_threads, 8, "The number of threads to spawn.");
DEFINE_int32(execution_time, 3, "The amount of time to run the tests");
DEFINE_int64(buffer_length, 65536, "The size of the region to test on.");
DEFINE_int32(operation_type, 0, "The type of test to execute"
    "(0: Both enqueue and dequeue"
    ", 1: Only Enqueue"
    ", 2: Only Dequeue)");

enum class TestType : size_t {ENQUEUEDEQUEUE = 0, ENQUEUE = 1, DEQUEUE = 2};

class TestObject {
 public:
  TestObject(int num_threads, int execution_time, int buffer_length,
             TestType test_type, RingBuffer<long> *ring_buffer)
      : execution_time_(execution_time)
      , buffer_length_(buffer_length)
      , num_threads_(num_threads)
      , operation_type_(test_type)
      , rb_(ring_buffer) {}

  void atomic_add(int enqueue_count, int dequeue_count) {
    enqueue_count_.fetch_add(enqueue_count);
    dequeue_count_.fetch_add(dequeue_count);
  }




  const int execution_time_;
  const int buffer_length_;
  const int num_threads_;
  const TestType operation_type_;

  RingBuffer<long> *rb_;

  std::atomic<uint64_t> enqueue_count_ {0};
  std::atomic<uint64_t> dequeue_count_ {0};
  std::atomic<void *>* shared_memory_;

  std::atomic<int> ready_count_ {0};
  std::atomic<bool> running_ {true};
  std::atomic<bool> wait_flag_ {true};
};


void run(int thread_id, tervel::Tervel* tervel_obj, TestObject * test_object);
void run_update_object(int start_pos, TestObject * test_data);

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tervel::Tervel tervel_obj(FLAGS_num_threads);
  tervel::ThreadContext tervel_thread(&tervel_obj);

  RingBuffer<long> *rb = new RingBuffer<long>(FLAGS_buffer_length);
  TestObject test_data(FLAGS_num_threads, FLAGS_execution_time,
        FLAGS_buffer_length, static_cast<TestType>(FLAGS_operation_type), rb);

  // prefill the buffer if needed
  if (test_data.operation_type_ == TestType::ENQUEUEDEQUEUE) {
    for (int i = 0; i < test_data.buffer_length_/2; i++) {
      rb->enqueue(i);
    }
    printf("Prefilled buffer to half capacity\n");
  }

  std::vector<std::thread> thread_list;
  for (int i = 0; i < test_data.num_threads_; i++) {
    std::thread temp_thread(run, i, &tervel_obj, &test_data);
    thread_list.push_back(std::move(temp_thread));
  }

  while (test_data.ready_count_.load() < test_data.num_threads_) {}

  printf("Beginning Test.\n");
  test_data.wait_flag_.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(test_data.execution_time_));
  test_data.running_.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  printf("Signaled Stop!\n");

  std::for_each(thread_list.begin(), thread_list.end(), [](std::thread &t) {
      t.join();
  });

  #if DEBUG
  rb->print_buffer_contents()
  rb->print_lost_nodes();
  #endif  // DEBUG

  printf("Completed[Enqueues: %lu, Dequeues: %lu]\n",
    test_data.enqueue_count_.load(), test_data.dequeue_count_.load());

  return 0;
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

  // Give a unique id number and add one to force an enqueue op to occur
  // first. This shift should make enqueues ID'ed for up to 256 threads.
  long val = (thread_id << 24) + 1;

  int op_mask = 0x1;
  while (test_data->running_.load()) {
    val++;
    if (val & op_mask) {
      bool succ = test_data->rb_->enqueue(-1*val);
      if (succ) {
        enqueue_count++;
      }
    } else {
      long res = -1;
      bool succ = test_data->rb_->dequeue(&res);
      if (succ) {
        assert(res != -1);
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

  long val = thread_id << 20;
  while (test_data->running_.load()) {
    val++;
    bool succ = test_data->rb_->enqueue(-1*val);
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

  long val = thread_id << 20;
  while (test_data->running_.load()) {
    val++;
    long res = -1;
    bool succ = test_data->rb_->dequeue(&res);
    if (succ) {
      assert(res != -1);
      dequeue_count++;
    }
  }  // End Execution Loop

  test_data->atomic_add(0, dequeue_count);
}
