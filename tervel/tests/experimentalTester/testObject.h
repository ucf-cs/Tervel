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


/** Arguments for Tester */
DEFINE_int32(num_threads, 1, "The number of executing threads.");
DEFINE_int32(execution_time, 5, "The amount of time to run the tests");


#define __tervel_xstr(s) __tervel_str(s)
#define __tervel_str(s) #s
#include __tervel_xstr(CONTAINER_FILE)
#undef __tervel_str
#undef __tervel_xstr


typedef struct{
  int fail_;
  int pass_;
  float rate_;
  void init(float rate) {
    fail_ = 0;
    pass_ = 0;
    rate_ = rate;
  };

  void inc(bool res) {
    if (res) {
      pass_++;
    } else {
      fail_ ++;
    }
  };

  int fail() {
    return fail_;
  };

  int pass() {
    return pass_;
  };

  float rate() {
    return rate_;
  };

}op_counter_t;


typedef int64_t Value;
class TestObject {
 public:
  TestObject()
      : num_threads_(FLAGS_num_threads)
      , execution_time_(FLAGS_execution_time) {

        test_results_ = new op_counter_t *[num_threads_];
      };

  ~TestObject() {
    DS_DESTORY_CODE
  };


  void extra_end_signal() {
    /* This function is useful for enabling a blocking operation to return*/
  };

  void sanity_check();

  void init() {
    DS_INIT_CODE
  }
  void destroy() {
    DS_DESTORY_CODE
  };

  void run(int64_t thread_id, char **argv) {
    // Initial Setup
    DS_ATTACH_THREAD

    op_counter_t *op_counter = new op_counter_t[DS_OP_COUNT];

    int lcount = 0;

    int func_call_rate[DS_OP_COUNT];
    int max_rand = 0;
    for (int i = 0; i < DS_OP_COUNT; i++) {
      int rate = atoi(argv[i]);
      op_counter[i].init(rate);
      func_call_rate[i] = rate + max_rand;
      max_rand = func_call_rate[i];
    }
    func_call_rate[DS_OP_COUNT-1]++;

    for (int i = 0; i < DS_OP_COUNT; i++) {
      op_counter[i].rate_ /= (float)(max_rand);
    }

    // Setup Random Number Generation
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(1, max_rand+1);

    OP_RAND

    // Wait for start signal
    ready_count_.fetch_add(1);
    while (wait_flag_.load());

    /** Update this when adding a new data structure **/
    while (running_.load()) {
      lcount++;
      int op = distribution(generator);

      bool opRes;

      #define MACRO_OP_SELECTOR(OP_ID, OP_CODE) \
        if (op <= func_call_rate[ OP_ID ]) { \
          OP_CODE \
          \
          op_counter[ OP_ID ].inc(opRes); \
          continue; \
        }

      MACRO_OP_SELECTOR(OP_0_ID, OP_0_CODE)
      MACRO_OP_SELECTOR(OP_1_ID, OP_1_CODE)

      assert(false);
    }
    ready_count_.fetch_add(1);

    test_results_[thread_id] = op_counter;

    DS_DETACH_THREAD
  };

  std::string results() {
    std::string res("");
    res += "Test Handler Configuration\n";
    res += "\tThreads:" + std::to_string(num_threads_) + "\n";
    res += "\tExecution Time: " + std::to_string(execution_time_) + "\n";

    res += "Test Class Configuration\n";
    res += DS_TO_STRING + "\n";


    res += "Thread Results?\n";
    res += "TID";
    for (int i = 0; i < DS_OP_COUNT; i++) {
      res += "\t" + op_names[i] + "-Pass";
      res += "\t" + op_names[i] + "-Fail";
      res += "\t" + op_names[i] + "-Rate";
    }
    res += "\n";

    int sum = 0;
    for (int i = 0; i < num_threads_; i++) {
      res += std::to_string(i);
      for (int j = 0; j < DS_OP_COUNT; j++) {
        int p = test_results_[i][j].pass();
        int f = test_results_[i][j].fail();
        float r = test_results_[i][j].rate();

        res += "\t" + std::to_string(p);
        res += "\t" + std::to_string(f);
        res += "\t" + std::to_string(r);

        sum = sum + p + f;
      }
      res += "\n";
    }

    res += "Total Operations: " + std::to_string(sum) + "\n";
    return res;
  }

  const int num_threads_;
  const int execution_time_;

  DS_DECLARE_CODE

  std::atomic<bool> wait_flag_{true};
  std::atomic<bool> running_{true};
  std::atomic<int> ready_count_{0};

  op_counter_t **test_results_;

  const std::string op_names[DS_OP_COUNT] = DS_OP_NAMES;
};


