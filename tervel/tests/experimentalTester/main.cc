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

#include <thread>
#include <time.h>
#include <vector>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <gflags/gflags.h>
#include <sys/time.h>
#include "testObject.h"

void run(TestObject *t, int id, char **argv) {
#ifdef USE_CDS
  cds::gc::HP::thread_gc myThreadGC (true) ;
#endif

  return t->run(id, argv);
};

int main(int argc, char **argv) {
#ifdef USE_CDS
  cds::Initialize() ;
{
  cds::gc::HP hpGC;
  cds::gc::HP::thread_gc myThreadGC (true) ;
#endif

  gflags::ParseCommandLineFlags(&argc, &argv, true);

  // Create Test Object
  TestObject test_data(argc, argv);

  test_data.init();

  // Create Threads
  std::vector<std::thread> thread_list;

  int64_t tid = 0;
  for (int j = 1; j < argc; j += DS_OP_COUNT + 1) {
    int t = atoi(argv[j]);
    for (int64_t i = 0; i < t; i++) {
      std::thread temp_thread(run, &test_data, tid++, &(argv[j+1]));
      thread_list.push_back(std::move(temp_thread));
    }
  };

  // Wait until Threads are ready
  while (test_data.ready_count_.load() < FLAGS_num_threads);

  std::this_thread::sleep_for(std::chrono::seconds(FLAGS_main_sleep));
  struct timeval start_time;
  (void)gettimeofday(&start_time, NULL);

// #ifdef DEBUG
//   printf("Debug: Beginning Test.\n");
// #endif
  test_data.wait_flag_.store(false);

  // Wait until test is over
  std::this_thread::sleep_for(std::chrono::seconds(test_data.execution_time_));
  test_data.ready_count_.store(0);

  // Signal Stop
  test_data.wait_flag_.store(true);
  test_data.running_.store(false);

  struct timeval end_time;
  (void)gettimeofday(&end_time, NULL);

// #ifdef DEBUG
//   printf("Debug: Signaled Stop!\n");
// #endif
  // Pause
  std::this_thread::sleep_for(std::chrono::seconds(1));

  test_data.extra_end_signal();

  // Wait until all threads are done.
  while (test_data.ready_count_.load() < FLAGS_num_threads);


  std::for_each(thread_list.begin(), thread_list.end(),
                [](std::thread &t) { t.join(); });

  // Print results
  test_data.set_start_time((double)start_time.tv_sec + (1.0/1000000) * (double)start_time.tv_usec);

  test_data.set_end_time((double)end_time.tv_sec + (1.0/1000000) * (double)end_time.tv_usec);

  std::cout << test_data.results() << std::endl;



  std::this_thread::sleep_for(std::chrono::seconds(1));

#ifdef USE_CDS
  }
#endif

  return 0;
}
