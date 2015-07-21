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
#ifndef __TESTERRUN_IMP_H
#define __TESTERRUN_IMP_H

void run(uint64_t thread_id, char **argv) {
#ifdef USE_CDS
  cds::gc::HP::thread_gc myThreadGC (true) ;
#endif
  {
    DS_ATTACH_THREAD
  }

  op_counter_t *op_counter = new op_counter_t[DS_OP_COUNT];
  g_test_results[thread_id] = op_counter;

  __attribute__((unused)) int lcount = 1;

  int func_call_rate[DS_OP_COUNT];
  int max_rand = 0;



  for (int i = 0; i < DS_OP_COUNT; i++) {
    int rate;
    if (FLAGS_iter_dist) {
      rate = 1;
    } else {
      rate = atoi(argv[i]);
    }
    op_counter[i].init(rate);
    func_call_rate[i] = rate + max_rand;
    max_rand = func_call_rate[i];
  }
  func_call_rate[DS_OP_COUNT-1]++;

  for (int i = 0; i < DS_OP_COUNT; i++) {
    op_counter[i].rate_ /= (float)(max_rand);
  }

  // Setup Random Number Generation
  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
  seed1 *= (thread_id + FLAGS_num_threads);
  std::default_random_engine generator(seed1);
  std::uniform_int_distribution<int> distribution(1, max_rand);
  const bool skip_ran = (1 == max_rand);

  OP_RAND

  // Wait for start signal
  g_thread_signal.ready();
  while (g_thread_signal.wait());

  /** Update this when adding a new data structure **/
  __attribute__((unused)) int op = DS_OP_COUNT % (1 + thread_id);
  while (g_thread_signal.execute()) {
    lcount++;

    if (FLAGS_iter_dist) {
      op--;
      if (op <= 0) {
        op = DS_OP_COUNT;
      }
    } else {
      op = (skip_ran) ? 1 : distribution(generator);
    }

    bool opRes = true;

    OP_CODE

    log("ERROR", " FALL Through on the thread's run while loop");
    exit(-1);
  }

  g_thread_signal.finished();

  {
    DS_DETACH_THREAD
  }
};

#endif  // #ifndef __TESTERRUN_IMP_H