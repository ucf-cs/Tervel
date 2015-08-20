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

#include <climits>
#include <assert.h>
#include <atomic>
#include <random>
#include <string>
#include <cstdint>
#include <stdio.h>
#include <iostream>
#include <gflags/gflags.h>

#define __tervel_xstr(s) __tervel_str(s)
#define __tervel_str(s) #s
#include __tervel_xstr(CONTAINER_FILE)
#undef __tervel_str
#undef __tervel_xstr

/**Change these when adapting to a new data structures **/

// Constructor Arguments
DEFINE_int32(capacity, 64, "The initial capacity of the container");

// Operation Rates

DEFINE_int32(prefill, 0, "The number of elements to be initially inserted.");
DEFINE_int32(enqueue_rate, 60, "The percent of insert operations.");
DEFINE_int32(dequeue_rate, 40, "The percent of find operations.");
const int32_t FLAGS_failenqueue_rate = 0;
const int32_t FLAGS_faildequeue_rate = 0;

/** Arguments for Tester */
DEFINE_int32(num_threads, 1, "The number of executing threads.");
DEFINE_int32(execution_time, 5, "The amount of time to run the tests");



typedef int64_t Value;
class TestObject {
 public:
  enum op_codes : int { enqueue = 0, dequeue, faildequeue, failenqueue, LENGTH };
  static const int k_num_functions = op_codes::LENGTH;

  int* func_call_rate_;
  std::string* func_name_;

  std::atomic<int> func_call_count_[k_num_functions];
  void set_rates() {
    func_call_rate_ = new int[k_num_functions];
    func_name_ = new std::string[k_num_functions];
    #define MACRO_ADD_RATE(TervelOpName) \
      func_name_[op_codes::TervelOpName] = "" #TervelOpName ; \
      func_call_rate_[op_codes::TervelOpName] = FLAGS_##TervelOpName##_rate; \
      func_call_count_[op_codes::TervelOpName].store(0);

    MACRO_ADD_RATE(enqueue)
    MACRO_ADD_RATE(dequeue)
    MACRO_ADD_RATE(failenqueue)
    MACRO_ADD_RATE(faildequeue)
  };

  TestObject()
      : num_threads_(FLAGS_num_threads)
      , execution_time_(FLAGS_execution_time) {
    set_rates();

    test_class_ = new TestClass<Value>(FLAGS_num_threads+1, FLAGS_capacity);


  };

  ~TestObject() {
    delete test_class_;
  };

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

  void sanity_check();

  void init() {
    /* This function is useful if you need to pre-fill a data structure */
    sanity_check();

    std::default_random_engine generator;
    std::uniform_int_distribution<uint64_t> largeValue(0, UINT_MAX);
    for (int i = 0; i < FLAGS_prefill; i++) {
      uint64_t x = largeValue(generator) & (~0x3);
      test_class_->enqueue(x);
    }
  }
  void destroy() {
    delete test_class_;
  };

  void run(int64_t thread_id) {
    // Initial Setup
    attachThread(thread_id);

    int lcount = 0;
    int ecount = 0;
    int func_call_count[k_num_functions];
    int func_call_rate[k_num_functions];
    int max_rand = 0;
    for (int i = 0; i < k_num_functions; i++) {
      func_call_rate[i] = func_call_rate_[i] + max_rand;
      max_rand = func_call_rate[i];
      func_call_count[i] = 0;
    }

    // Setup Random Number Generation
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, max_rand);
    std::uniform_int_distribution<int> largeValue(0, UINT_MAX);

    // Wait for start signal
    ready_count_.fetch_add(1);
    while (wait_flag_.load());

    /** Update this when adding a new data structure **/
    while (running_.load()) {
      lcount++;
      int op = distribution(generator);

      if (op <= func_call_rate[op_codes::enqueue]) {
        // Value x = largeValue(generator) & (~0x3);
        Value x = (thread_id << 56) | ecount;
        if (test_class_->enqueue(x)) {
          func_call_count[op_codes::enqueue]++;
          ecount++;
        } else {
          func_call_count[op_codes::failenqueue]++;
        }
      } else if (op <= func_call_rate[op_codes::dequeue]) {
        Value x;
        if (test_class_->dequeue(x)) {
          func_call_count[op_codes::dequeue]++;
        } else {
          func_call_count[op_codes::faildequeue]++;
        }
      } else {
        assert(false);
      }
    }
    ready_count_.fetch_add(1);

    add_results(func_call_count);
    detachThread(thread_id);
  };

  std::string toString() {
    std::string res("");
    res += "Test Handler Configuration\n";
    res += "\tThreads:" + std::to_string(num_threads_) + "\n";
    res += "\tExecution Time: " + std::to_string(execution_time_) + "\n";

    res += "\tOperation rates\n";
    for (int i = 0; i < k_num_functions; i++) {
      res += "\t\t" + func_name_[i] + ": ";
      res += std::to_string(func_call_rate_[i]) + "\n";
    }

    return res;
  };

  std::string results() {
    std::string res("");
    res += "-- Test Results--\n";
    res += this->toString() + "\n";

    res += "Test Class Configuration\n";
    res += test_class_->toString() + "\n";


    res += "Operation Counts\n";
    int sum = 0;
    for (int i = 0; i < k_num_functions; i++) {
      res += "\t" + func_name_[i] + ": " +
             std::to_string(func_call_count_[i].load()) + "\n";
      sum += func_call_count_[i].load();
    }
    res += "Total Operations: " + std::to_string(sum) + "\n";
    return res;
  }

  void add_results(int func_call_count[k_num_functions]) {
    for (int i = 0; i < k_num_functions; i++) {
      func_call_count_[i].fetch_add(func_call_count[i]);
    }
  }

  const int num_threads_;
  const int execution_time_;

  TestClass<Value> *test_class_;

  std::atomic<bool> wait_flag_{true};
  std::atomic<bool> running_{true};
  std::atomic<int> ready_count_{0};
};


void TestObject::
sanity_check() {
  bool res;
  Value i, j, temp;

  for (i = 0; i < FLAGS_capacity; i++) {
    bool res = test_class_->enqueue(i);
    assert(res && "If this assert fails then the there is an issue with either pushing or determining that it is full");
  };

  res = test_class_->enqueue(0xDEADBEEF);
  assert(!res && "If this assert fails then the there is an issue with either pushing or determining that it is full");

  for (j = 0; j < FLAGS_capacity; j++) {
    res = test_class_->dequeue(temp);
    assert(res && "If this assert fails then the there is an issue with either poping or determining that it is not empty");
    assert(temp==j && "If this assert fails then there is an issue with  determining the top element");
  };

  res = test_class_->dequeue(temp);
  assert(!res && "If this assert fails then there is an issue with pop or determining that it is empty");
};