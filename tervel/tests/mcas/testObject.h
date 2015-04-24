/*
#The MIT License (MIT)
#
#Copyright (c) 2015 University of Central Florida's Computer Software Engineering
#Scalable & Secure Systems (CSE - S3) Lab
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.
#
*/

#include <functional>
#include <climits>
#include <assert.h>
#include <atomic>
#include <random>
#include <string>
#include <cstdint>
#include <stdio.h>
#include <iostream>
#include <gflags/gflags.h>

/** Change this when including a new data structure */
#include <tervel/tests/mcas/container_api.h>

/**Change these when adapting to a new data structures **/

// Constructor Arguments
DEFINE_int32(array_length, 32, "The size of the region to test on.");
DEFINE_int32(mcas_size, 2, "The number of words in a mcas operation.");

/** Operation Parameters **/
DEFINE_int32(operation_type, 0, "The type of test to execute"
    "(0: updating single multi-word object"
    ", 1: updating multiple objects"
    ", 2: updating overlapping multi-word updates)");

enum class TestType : size_t {UPDATEOBJECT = 0, UPDATEMULTIOBJECT = 1,
      RANDOMOVERLAPS = 2};


/** Arguments for Tester */
DEFINE_int32(num_threads, 1, "The number of threads to spawn.");
DEFINE_int32(execution_time, 5, "The amount of time to run the tests");

typedef uint64_t Value;
class TestObject {
 public:


  ~TestObject() {
    delete test_class_;
  };

  TestObject()
      : execution_time_(FLAGS_execution_time)
      , array_length_(FLAGS_array_length)
      , num_threads_(FLAGS_num_threads)
      , mcas_size_(FLAGS_mcas_size)
      , operation_type_(FLAGS_operation_type) {
        shared_memory_ = new std::atomic<void *>[array_length_];
        passed_count_.store(0);
        failed_count_.store(0);

        test_class_ = new TestClass(num_threads_+1);
      }

  void atomic_add(int passed_count, int failed_count) {
    passed_count_.fetch_add(passed_count);
    failed_count_.fetch_add(failed_count);
  }



  // Define these functions if the test object requires per thread actions
  void attachThread(int threadID) {
    test_class_->attach_thread();
  };

  void detachThread(int threadID) {
    test_class_->detach_thread();
  };

  void extra_end_signal() {
    /* This function is useful for enabling a blocking operation to return*/
  };

  void init() {
    /* This function is useful if you need to pre-fill a data structure */
  }
  void destroy() {
    delete test_class_;
  };

  struct counter{
    int inc() {
      assert(x_ >= 0 && x_ < limit_);
      int y = x_++;
      y += offset_;
      assert(y >= 0 && y < limit_);
      return y;
    };

    void reset1(int offset, int len) {
      offset_ = offset * len;
      x_ = 0;
    };

    void reset2(std::function<int()> func, int len) {
      offset_ = func() * len;
      x_ = 0;
    };

    void setLimit(int l) {
      limit_ = l;
    };

    int limit_ = 0;
    int x_ = 0;
    int offset_ = 0;
  };

  void run(int64_t thread_id) {
    // Initial Setup
    int passed_count = 0;
    int failed_count = 0;
    int lcount = 0;
    attachThread(thread_id);

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, array_length_);
    std::uniform_int_distribution<int> dist_objs(0, array_length_/mcas_size_);

    struct counter count;
    count.setLimit(array_length_);

    std::function<int()> posFunc;
    std::function<void()> resFunc;
    if (FLAGS_operation_type == 0) {
      posFunc = std::bind(&counter::inc, count);
      resFunc = std::bind(&counter::reset1, count,
        0, mcas_size_);
    } else if (FLAGS_operation_type == 1) {
      posFunc = std::bind(&counter::inc, count);

      std::function<int()> func = std::bind(dist_objs, generator);
      resFunc = std::bind(&counter::reset2, count,
        func, mcas_size_);
    } else if (FLAGS_operation_type == 2) {
      posFunc = std::bind(distribution, generator);
      resFunc = std::bind(&counter::reset1, count,
        0, mcas_size_);
    } else {
      assert(false && "Operation type not recognized");
    }

    // Wait for start signle
    ready_count_.fetch_add(1);
    while (wait_flag_.load());

    /** Update this when adding a new data structure **/
    while (running_.load()) {
      lcount++;
      resFunc();

      bool res = test_class_->mcas(mcas_size_, posFunc, shared_memory_);
      if (res) {
        passed_count++;
      } else {
        failed_count_++;
      }

    }
    ready_count_.fetch_add(1);

    atomic_add(passed_count, failed_count);
    detachThread(thread_id);
  };

  std::string toString() {
    std::string res("");
    res += "Test Handler Configuration\n";
    res += "\tThreads:" + std::to_string(num_threads_) + "\n";
    res += "\tExecution Time: " + std::to_string(execution_time_) + "\n";
    res += "\tarray_length: " + std::to_string(array_length_)+"\n";
    res += "\tmcas_size: " + std::to_string(mcas_size_)+"\n";
    res += "\toperation_type: " + std::to_string(operation_type_)+"\n";
    return res;
  };

  std::string results() {
    uint64_t p = passed_count_.load();
    uint64_t f = failed_count_.load();
    std::string res("");
    res += "-- Test Results--\n";
    res += this->toString() + "\n";
    res += "\tSuccesses: " + std::to_string(p)+"\n";
    res += "\tFailures: " + std::to_string(f)+"\n";
    res += "\tTotal: " + std::to_string(p+f)+"\n";
    return res;
  };



  const int execution_time_;
  const int array_length_;
  const int num_threads_;
  const int mcas_size_;
  const int operation_type_;

  TestClass * test_class_;

  std::atomic<uint64_t> passed_count_ {0};
  std::atomic<uint64_t> failed_count_ {0};
  std::atomic<void *>* shared_memory_;

  std::atomic<bool> wait_flag_{true};
  std::atomic<bool> running_{true};
  std::atomic<int> ready_count_{0};
};
