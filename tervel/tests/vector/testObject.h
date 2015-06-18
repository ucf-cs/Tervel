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

/** Change this when including a new data structure */
#include <tervel/tests/vector/container_api.h>

/**Change these when adapting to a new data structures **/

// Constructor Arguments
DEFINE_int32(capacity, 64, "The initial capacity of the vector");

// Operation Rates
DEFINE_int32(cas_rate, 0,
             "The chance (0-100) of the CAS operation being called.");
DEFINE_int32(at_rate, 0,
             "The chance (0-100) of the at operation being called.");
DEFINE_int32(pushBack_rate, 0,
             "The chance (0-100) of pushBack operation being called.");
DEFINE_int32(popBack_rate, 0,
             "The chance (0-100) of popBack operation being called.");
DEFINE_int32(size_rate, 0,
             "The chance (0-100) of size operation being called.");
DEFINE_int32(insertAt_rate, 0,
             "The chance (0-100) of insertAt operation being called.");
DEFINE_int32(eraseAt_rate, 0,
             "The chance (0-100) of eraseAt operation being called.");

/** Arguments for Tester */
DEFINE_int32(num_threads, 1, "The number of executing threads.");
DEFINE_int32(execution_time, 5, "The amount of time to run the tests");

typedef int64_t Value;
class TestObject {
 public:
  enum op_codes : int { cas = 0, at, popBack, pushBack, size, insertAt, eraseAt, LENGTH };
  static const int k_num_functions = op_codes::LENGTH;
  int* func_call_rate_;
  std::string* func_name_;

  std::atomic<int> func_call_count_[k_num_functions];
  void set_rates() {
    func_call_rate_ = new int[k_num_functions];
    func_name_ = new std::string[k_num_functions];

    #define MACRO_ADD_RATE(TervelOpName) \
      func_name_[op_codes::TervelOpName] = "" #TervelOpName ; \
      func_call_rate_[op_codes::TervelOpName] = FLAGS_##TervelOpName##_rate;

    MACRO_ADD_RATE(cas)
    MACRO_ADD_RATE(pushBack)
    MACRO_ADD_RATE(popBack)
    MACRO_ADD_RATE(size)
    MACRO_ADD_RATE(at)
    MACRO_ADD_RATE(insertAt)
    MACRO_ADD_RATE(eraseAt)


    for (int i = 0; i < k_num_functions; i++) {
      func_call_count_[i].store(0);
    }
  };

  TestObject()
      : num_threads_(FLAGS_num_threads),
        execution_time_(FLAGS_execution_time),
        test_class_(new TestClass<Value>( (num_threads_+1), FLAGS_capacity)) {
    set_rates();
    for (int i = 0; i < FLAGS_capacity/2; i++) {
      test_class_->push_back( (1 + i) * 0x8 );
      func_call_count_[op_codes::pushBack]++;
    }
  };

  void print_vector() {
    std::cout << "Vector's contents: " << std::endl;
    for (size_t i = 0; i < test_class_->size() + 100; i++) {
      Value temp = 0;
      test_class_->at(i, temp);
      uint64_t tid = temp >>  (sizeof(Value)*8-7);
      uint64_t c = (temp << 7) >> (7+3);
      std::cout << "\t[" << i << "] TID: " << tid << " C: " << c << std::endl;
    }
  }
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

  void init() {
    /* This function is useful if you need to pre-fill a data structure */
  }
  void destroy() {
    delete test_class_;
  };

  void run(int64_t thread_id) {
    // Initial Setup
    attachThread(thread_id);

    int lcount = 1;
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
    std::uniform_int_distribution<int> distribution(1, max_rand);
    std::uniform_int_distribution<int> largeValue(0, UINT_MAX);

    // Wait for start signle
    ready_count_.fetch_add(1);
    while (wait_flag_.load());

    /** Update this when adding a new data structure **/
    while (running_.load()) {
      lcount++;
      int op = distribution(generator);
      if (op <= func_call_rate[op_codes::cas]) {
        size_t s = test_class_->size();
        if (s == 0) {
          continue;
        }
        Value temp;
        int idx = largeValue(generator) % s;
        if (test_class_->at(idx, temp)) {
          test_class_->cas(idx, temp, (temp * 2) & (~0x7));
          func_call_count[op_codes::cas]++;
        }
      } else if (op <= func_call_rate[op_codes::at]) {
        size_t s = test_class_->size();
        if (s == 0) {
          continue;
        }

        Value temp;
        size_t idx = largeValue(generator) % s;
        test_class_->at(idx, temp);

        func_call_count[op_codes::at]++;
      } else if (op <= func_call_rate[op_codes::popBack]) {
        Value temp;
        if (test_class_->pop_back(temp))
          func_call_count[op_codes::popBack]++;
      } else if (op <= func_call_rate[op_codes::pushBack]) {
        Value temp = reinterpret_cast<Value>(thread_id);
        temp = temp << (sizeof(Value)*8-7);
        temp = temp | (lcount << 3);

        test_class_->push_back(temp);
        func_call_count[op_codes::pushBack]++;
      } else if (op <= func_call_rate[op_codes::size]) {
        test_class_->size();
        func_call_count[op_codes::size]++;
      } else if (op <= func_call_rate[op_codes::insertAt]) {
        size_t s = test_class_->size();
        if (s == 0) {
          continue;
        }

        Value temp = reinterpret_cast<Value>(thread_id);
        temp = temp << (sizeof(Value)*8-7);
        temp = temp | (lcount << 3);

        size_t idx = largeValue(generator) % s;

        if (test_class_->insertAt(idx, temp)) {
          func_call_count[op_codes::insertAt]++;
        }
      } else if (op <= func_call_rate[op_codes::eraseAt]) {
        size_t s = test_class_->size();
        if (s == 0) {
          continue;
        }

        Value temp;
        size_t idx = largeValue(generator) % s;

        if (test_class_->eraseAt(idx, temp)) {
          func_call_count[op_codes::eraseAt]++;
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
    res += "Reported Size: " + std::to_string(test_class_->size()) + "\n";
    {
      int temp = func_call_count_[op_codes::pushBack] - func_call_count_[op_codes::popBack];
      temp += func_call_count_[op_codes::insertAt] - func_call_count_[op_codes::eraseAt];
      res += "Calculated Size: " + std::to_string(temp) + "\n";
    }
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
