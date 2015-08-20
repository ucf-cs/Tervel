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

void run(TestObject *t, int id, PapiUtil *papiUtil, long long *counts) {
#ifdef USE_CDS
  cds::gc::HP::thread_gc myThreadGC (true) ;
#endif


 

  int retval = PAPI_register_thread(  );
  if ( retval != PAPI_OK ) {
     std::cout << "PAPI ERROR (PAPI_register_thread) ! ( " << retval << " )" << std::endl;
  }

  //PAPI start
  std::vector<int> availableEvents;
  bool result = papiUtil->start(papiUtil->cacheInfo[FLAGS_PAPI_metricCacheInfo],availableEvents);
  if(!result)
    std::cout << "PAPI ERROR (start) !" << std::endl;
  for(unsigned int i = 0; i < availableEvents.size(); i++){
    counts[i] = 0;
  } 
  
  t->run(id);

  // PAPI stop
  papiUtil->stop(counts,availableEvents.size());

  // print PAPI results
  // papiUtil->printResults(availableEvents,counts);  

  retval = 0;
  retval = PAPI_unregister_thread(  );
  if ( retval != PAPI_OK )
    std::cout << "PAPI ERROR (PAPI_unregister_thread) ! ( " << retval << " )" << std::endl;
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
  
  PapiUtil papiUtil;
  papiUtil.init();
  long long **threadCounters = new long long *[FLAGS_num_threads];
  std::vector<int> availableEvents = papiUtil.filterAvailableEvents(papiUtil.cacheInfo[FLAGS_PAPI_metricCacheInfo]);  
  // Create Threads
  std::vector<std::thread> thread_list;
  for (int64_t i = 0; i < FLAGS_num_threads; i++) {
    threadCounters[i] = new long long[availableEvents.size()];
    std::thread temp_thread(run, &test_data, i, &papiUtil, threadCounters[i]);
    thread_list.push_back(std::move(temp_thread));
  }

  // Wait until Threads are ready
  while (test_data.ready_count_.load() < FLAGS_num_threads);

#ifdef DEBUG
  printf("Debug: Beginning Test.\n");
#endif



  std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

  test_data.wait_flag_.store(false);

  // Wait until test is over
  // while(test_data.visitedCounter < 100000){
  //   std::cout << "main : visitedCounter= " << test_data.visitedCounter << std::endl;
  //   std::this_thread::sleep_for(std::chrono::milliseconds(500));
  // }
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
  // std::cout << "before waiting for threads" << test_data.ready_count_.load() << std::endl;
  while (test_data.ready_count_.load() < FLAGS_num_threads);
  // std::cout << "after waiting for threads: " << test_data.ready_count_.load() << std::endl;
  std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
  


  std::for_each(thread_list.begin(), thread_list.end(),
                [](std::thread &t) { t.join(); });


  // Print results
  std::cout << test_data.results() << std::endl;
  unsigned long execTime = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
  std::cout << "execTime: " << execTime << std::endl;


  long long *counts = new long long[availableEvents.size()];
  for (unsigned int i = 0; i < availableEvents.size(); ++i)
  {
    counts[i] = 0;
  }
  for (int64_t i = 0; i < FLAGS_num_threads; i++) {
    for(unsigned int j = 0; j < availableEvents.size(); j++){
      counts[j] += threadCounters[i][j];
    }
  }
  papiUtil.printResults(availableEvents,counts);  



  papiUtil.shutdown();



  std::this_thread::sleep_for(std::chrono::seconds(1));

#ifdef USE_CDS
  }
#endif

  return 0;
}
