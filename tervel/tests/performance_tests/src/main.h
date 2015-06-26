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
#ifndef __TESTER_MAIN_H
#define __TESTER_MAIN_H

#include <algorithm>
#include <atomic>
#include <climits>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <sys/time.h>
#include <vector>
#include <gflags/gflags.h>

#define __TERVEL_MACRO_xstr(s) __TERVEL_MACRO_str(s)
#define __TERVEL_MACRO_str(s) #s
#define _DS_CONFIG_INDENT "    "
#define DS_EXTRA_END_SIGNAL

#define MACRO_OP_MAKER(opid, opcode) \
  if (op <= func_call_rate[ opid ]) { \
    opcode \
    \
    op_counter[ opid ].inc(opRes); \
    continue; \
  } \
  \


#include __TERVEL_MACRO_xstr( ../ CONTAINER_FILE)

DEFINE_bool(verbose, false, "If true then verbose output is used");


std::string config_str(int numThreads, std::string execution_str);
std::string results_str(double start_time, double end_time, int numThreads);
void run(uint64_t id, char **argv);

typedef struct{
  std::atomic<bool> wait_;
  std::atomic<bool> execute_;
  std::atomic<uint64_t> ready_count_;
  std::atomic<uint64_t> finished_count_;

  void init() {
    wait_.store(true);
    execute_.store(true);
    ready_count_.store(0);
    finished_count_.store(0);
  }
  void start() {
    wait_.store(false);
  }
  void stop() {
    execute_.store(false);
  }
  bool wait() { return wait_.load(); };
  bool execute() { return execute_.load(); };

  bool notReady(uint64_t threads) {
    return ready_count_.load() < threads;
  };

  bool notFinished(uint64_t threads) {
    return finished_count_.load() < threads;
  };
  void ready() { ready_count_.fetch_add(1); };
  void finished() { finished_count_.fetch_add(1); };

} ThreadSignal;

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


void log(const char * tag, std::string msg, bool print = false) {
  if (FLAGS_verbose || print) {
    std::cout << "# "
      << "[" << tag << "]"
      << " : " << msg << std::endl;
  }
};
void log(const char * tag, const char * msg, bool print = false) {
  if (FLAGS_verbose || print) {
    std::cout << "# "
      << "[" << tag << "]"
      << " : " << msg << std::endl;
  }
};


void error_log(std::string msg) {
  log("Error", msg, true);
};

void sleep(int s) {
  std::this_thread::sleep_for(std::chrono::seconds(s));
};

#endif // #ifndef __TESTER_MAIN_H