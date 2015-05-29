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

#include "testObject.h"
#include "PapiUtil.h"

void run(TestObject *t, int id) {
#ifdef USE_CDS
  cds::gc::HP::thread_gc myThreadGC (true) ;
#endif

  return t->run(id);
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
  TestObject test_data;

  test_data.init();

  // Create Threads
  std::vector<std::thread> thread_list;
  for (int64_t i = 0; i < FLAGS_num_threads; i++) {
    std::thread temp_thread(run, &test_data, i);
    thread_list.push_back(std::move(temp_thread));
  }

  // Wait until Threads are ready
  while (test_data.ready_count_.load() < FLAGS_num_threads);

#ifdef DEBUG
  printf("Debug: Beginning Test.\n");
#endif

  //PAPI start
  PapiUtil papiUtil;
  std::vector<int> availableEvents;
  bool result = papiUtil.start(papiUtil.CacheDataAHM,availableEvents);
  if(!result)
    std::cout << "PAPI ERROR !" << std::endl;
  long long *counts = new long long[availableEvents.size()];

  test_data.wait_flag_.store(false);

  // Wait until test is over
  std::this_thread::sleep_for(std::chrono::seconds(test_data.execution_time_));
  test_data.ready_count_.store(0);

  // Signal Stop
  test_data.wait_flag_.store(true);
  test_data.running_.store(false);


#ifdef DEBUG
  printf("Debug: Signaled Stop!\n");
#endif
  // Pause
  std::this_thread::sleep_for(std::chrono::seconds(1));

  test_data.extra_end_signal();

  // Wait until all threads are done.
  while (test_data.ready_count_.load() < FLAGS_num_threads);

  // PAPI stop
  papiUtil.stop(counts,availableEvents.size());


  std::for_each(thread_list.begin(), thread_list.end(),
                [](std::thread &t) { t.join(); });


  // Print results
  std::cout << test_data.results() << std::endl;

  // print PAPI results
  papiUtil.printResults(availableEvents,counts);

  std::this_thread::sleep_for(std::chrono::seconds(1));

#ifdef USE_CDS
  }
#endif

  return 0;
}
