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
#include "main.h"

/** Arguments for Tester */
DEFINE_uint64(main_sleep, 0, "Causes the main thread to sleep before signaling go. Useful for allowing monitors to be attached.");
DEFINE_uint64(num_threads, 0, "The number of executing threads. The trailing arguments should be in the forum of thread groups which are in the form: [threads oprate1 .. oprateN]");
DEFINE_uint64(execution_time, 5, "The amount of time to run the tests");


// Global Variables
ThreadSignal g_thread_signal;

DS_DECLARE_CODE

const std::string op_names[DS_OP_COUNT] = { DS_OP_NAMES };
op_counter_t ** g_test_results;

int main(int argc, char **argv) {
#ifdef USE_CDS
  cds::Initialize() ;
{
  cds::gc::HP hpGC;
  cds::gc::HP::thread_gc myThreadGC (true) ;
#endif

  // Variables:
  struct timeval start_time, end_time;
  uint64_t numThreads = 0;

  gflags::ParseCommandLineFlags(&argc, &argv, true);

  {
    log("Info", "Initializing Tester Object and Data Structure");
    DS_INIT_CODE
    log("Info", "Completed Tester Object and Data Structure");
  }

  g_test_results = new op_counter_t *[FLAGS_num_threads];

  std::string execution_str = "";
  for (int i = 1; i < argc; i++) {
    execution_str += std::to_string(atoi(argv[i])) + " ";
  }


  if (FLAGS_seq_test) {
    log("Info", "Performing Sequential Sanity Check");
    sanity_check(container);
    log("Info", "Sequential Sanity Check Complete");
    exit(0);
  }

  // Create PAPI Objects
  #ifdef USE_PAPI
    PapiUtil papiUtil;
  #endif

  // Create Threads
  g_thread_signal.init();
  std::vector<std::thread> thread_list;


  for (int j = 1; j < argc; j += DS_OP_COUNT + 1) {

    if (j + DS_OP_COUNT >= argc) {
      error_log("Invalid Thread Group and Rate Configuration");
      std::string s = "Use: int values in the following order threads";
      for (int i = 0; i < DS_OP_COUNT; i++) {
        s += ", " + op_names[i];
      }
      error_log(s);
      exit(-1);
    }

    int t = atoi(argv[j]);
    for (int64_t i = 0; i < t; i++) {
      std::thread temp_thread(run, numThreads++, &(argv[j+1]));
      thread_list.push_back(std::move(temp_thread));
    }
  };

  if (FLAGS_num_threads < numThreads) {
    log("Error", "Specified num_threads is greater than the number of threads specified in the thread groups");
    exit(-1);
  }
  std::cout << config_str(numThreads, execution_str) << std::endl;
  sleep_wrapper(FLAGS_main_sleep);


  // Wait until Threads are ready
  while (g_thread_signal.notReady(numThreads));

  log("Info", "Threads Ready, beginning test.");


  (void)gettimeofday(&start_time, NULL);
#ifdef USE_PAPI
  papiUtil.start();
#endif
  g_thread_signal.start();

  // Wait until test is over
  sleep_wrapper(FLAGS_execution_time);

  g_thread_signal.stop();
  (void)gettimeofday(&end_time, NULL);

#ifdef USE_PAPI
  papiUtil.stop();
#endif

  log("Info", "Testing Completed");
  sleep_wrapper(1);

  DS_EXTRA_END_SIGNAL;

  // Wait until all threads are done.
  while (g_thread_signal.notFinished(numThreads));

  std::for_each(thread_list.begin(), thread_list.end(),
                [](std::thread &t) { t.join(); });

  std::string run_results = results_str(
    ( (double)start_time.tv_sec + (1.0/1000000) * (double)start_time.tv_usec ),
    ( (double)end_time.tv_sec   + (1.0/1000000) * (double)end_time.tv_usec   ),
    numThreads,
    container
  );
  std::cout << run_results << std::endl;

#ifdef USE_PAPI
  std::cout << papiUtil.results() << std::endl;
#endif

  sleep_wrapper(1);
  DS_DESTORY_CODE

#ifdef USE_CDS
  }
#endif
  log("Info", "FIN");
  return 0;
};

#include "run_imp.h"
#include "yaml_output.h"




