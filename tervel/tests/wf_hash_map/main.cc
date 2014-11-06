//
//  main_tester.c
//  mCAS
//
//  Created by Steven Feldman on 11/1/12.

//  Copyright (c) 2012 Steven FELDMAN. All rights reserved.
//

// #define KILL_THREAD 1
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/mman.h>


#include <atomic>
#include <vector>
#include <thread>
#include <iostream>

#include <gflags/gflags.h>

#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>

#include "container_api.h"

DEFINE_int32(capacity, 64, "The initial capacity of the hash map");
DEFINE_int32(num_threads, 1, "The number of executing threads.");
DEFINE_int32(execution_time, 1, "The length of time to execute tests for.");
DEFINE_int32(prefill, 0, "The number of elements to be initially inserted.");
DEFINE_int32(insert_rate, 25, "The percent of insert operations.");
DEFINE_int32(find_rate, 25, "The percent of find operations.");
DEFINE_int32(update_rate, 25, "The percent of update operations.");
DEFINE_int32(remove_rate, 25, "The percent of remove operations.");

class TestObject {
 public:
  TestObject(int num_threads = FLAGS_num_threads,
    int capacity = FLAGS_capacity,
    int execution_time = FLAGS_execution_time,
    size_t find_rate = FLAGS_find_rate,
    size_t insert_rate = FLAGS_insert_rate,
    size_t update_rate = FLAGS_update_rate,
    size_t remove_rate = FLAGS_remove_rate)
      : test_class_(num_threads, capacity)
      , num_threads_(num_threads)
      , execution_time_(execution_time)
      , find_rate_(find_rate)
      , insert_rate_(insert_rate)
      , update_rate_(update_rate)
      , remove_rate_(remove_rate) {}


  void print_test_info() {
    std::cout
      << "Test Results:" << std::endl
      << "\tAlgorithm = " << std::string(test_class_.name()) << std::endl
      << "\tNumber of Threads = " << FLAGS_num_threads << std::endl
      << "\tExecution Time = " << execution_time_ << std::endl
      << "\tPrefill % = " << FLAGS_prefill << std::endl
      << "\tFind Rate = " << find_rate_ << std::endl
      << "\tInsert Rate = " << insert_rate_ << std::endl
      << "\tUpdate Rate = " << update_rate_ << std::endl
      << "\tRemove Rate = " << remove_rate_ << std::endl
      << std::flush;
  }

  void print_results() {
    std::cout
      << "\tFind Count = " << afcount.load() << std::endl
      << "\tInsert Count = " << aicount.load() << std::endl
      << "\tUpdate Count = " << aucount.load() << std::endl
      << "\tRemove Count = " << arcount.load() << std::endl
      << "-------------------/proc/meminfo-----------------------" << std::endl
      << std::flush;

    system("cat /proc/meminfo");
    std::cout << "-------------------fin-----------------------"
      << std::endl << std::flush;
  }

  void thread_print_results(size_t thread_id, size_t fcount, size_t icount,
    size_t ucount, size_t rcount) {
    std::cout << "Thread " << thread_id << std::endl
      << "\tFind Count = " << fcount << std::endl
      << "\tInsert Count = " << icount << std::endl
      << "\tUpdate Count = " << ucount << std::endl
      << "\tRemove Count = " << rcount << std::endl
       << "-------------------/proc/self/status-----------------------" << std::endl
      << std::flush;

    system("cat /proc/self/status");
    std::cout << "-------------------fin-----------------------"
      << std::endl << std::endl << std::flush;
  }

  void update_results(size_t fcount, size_t icount, size_t ucount,
    size_t rcount) {
    afcount.fetch_add(fcount);
    aicount.fetch_add(icount);
    aucount.fetch_add(ucount);
    arcount.fetch_add(rcount);
  };


  TestClass<int64_t, int64_t> test_class_;
  const int num_threads_;
  const int execution_time_;

  size_t find_rate_;
  size_t insert_rate_;
  size_t update_rate_;
  size_t remove_rate_;


  std::atomic<int> ready_count_{0};
  std::atomic<bool> running_{true};
  std::atomic<bool> wait_flag_{true};
  std::atomic<size_t> afcount{0};
  std::atomic<size_t> aicount{0};
  std::atomic<size_t> aucount{0};
  std::atomic<size_t> arcount{0};
};


void run(int thread_id, TestObject * test_object);
void run_correctness(int thread_id, TestObject * test_data);

int main(int argc, char** argv) {
#ifdef USE_CDS
  cds::Initialize() ;
{
  cds::gc::HP hpGC;
  cds::gc::HP::thread_gc myThreadGC (true) ;
#endif

  gflags::ParseCommandLineFlags(&argc, &argv, true);

  TestObject test_data(FLAGS_num_threads+1, FLAGS_capacity,
      FLAGS_execution_time);

  test_data.print_test_info();

{
  boost::mt19937 rng(-1);
  boost::uniform_int<> brandValues(1, std::numeric_limits<int>::max());
  int limit = (int)((float)(FLAGS_prefill)/100.0 * FLAGS_capacity);
  for (int i = 0; i < limit; i++) {
    int64_t key = (int64_t)brandValues(rng) >> 15;
    test_data.test_class_.insert(key, key);
  }

}

  std::vector<std::thread> thread_list;
  for (int64_t i = 0; i < FLAGS_num_threads; i++) {
    #ifdef DEBUG
      std::thread temp_thread(run_correctness, i, &test_data);
    #else
      std::thread temp_thread(run, i, &test_data);
    #endif
    thread_list.push_back(std::move(temp_thread));
  }

  while (test_data.ready_count_.load() < FLAGS_num_threads) {}
  test_data.ready_count_.store(0);
#ifdef DEBUG
  printf("Beginning Debug Test.\n");
#endif

  test_data.wait_flag_.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(test_data.execution_time_));
  test_data.running_.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(1));

#ifdef DEBUG
  printf("Signaled Stop!\n");
#else

  while (test_data.ready_count_.load() < FLAGS_num_threads) {}

  test_data.print_results();

  test_data.ready_count_.store(0);
#endif

  std::for_each(thread_list.begin(), thread_list.end(), [](std::thread &t)
    { t.join(); });

#ifdef USE_CDS
  }
#endif

  return 0;
}

//

void run(int thread_id, TestObject * test_data) {

#ifdef USE_CDS
  cds::gc::HP::thread_gc myThreadGC (true) ;
#endif
  test_data->test_class_.attach_thread();

  boost::mt19937 rng(thread_id);
  boost::uniform_int<> brandValues(2, std::numeric_limits<int>::max());
  boost::uniform_int<> brandOperations(1, 100);

  const int frate = test_data->find_rate_;
  const int irate = frate + test_data->insert_rate_;
  const int urate = irate + test_data->update_rate_;
  const int rrate = urate + test_data->remove_rate_;

  size_t fcount = 0;
  size_t icount = 0;
  size_t ucount = 0;
  size_t rcount = 0;

  assert(rrate == 100);


  test_data->ready_count_.fetch_add(1);
  while (test_data->wait_flag_.load());

  while (test_data->running_.load()) {
    int op = (int)brandOperations(rng);
    int64_t key = (int64_t)brandValues(rng) & (0x0FFFFFFFFFFFFFF0);


    if (op <= frate) {
      int64_t temp = -1;
      test_data->test_class_.find(key, temp);
      fcount++;
    } else if (op <= irate) {
      test_data->test_class_.insert(key, key);
      icount++;
    } else if (op <= urate) {
      int64_t new_value = -1;
      test_data->test_class_.find(key, new_value);
      test_data->test_class_.update(key, new_value, new_value+1000);
      // fcount++;
      ucount++;
    } else if (op <= rrate) {
      test_data->test_class_.remove(key);
      rcount++;
    } else {
      assert(false);
    }
  }  // end while running

  test_data->test_class_.detach_thread();

  test_data->update_results(fcount, icount, ucount, rcount);
  test_data->ready_count_.fetch_add(1);

  while (test_data->ready_count_.load() < FLAGS_num_threads);

  while (test_data->ready_count_.load() != thread_id);
  test_data->thread_print_results(thread_id, fcount, icount, ucount, rcount);
  test_data->ready_count_.fetch_add(1);
}


void run_correctness(int thread_id, TestObject * test_data) {

#ifdef USE_CDS
  cds::gc::HP::thread_gc myThreadGC (true) ;
#endif


  const int64_t num_threads = test_data->num_threads_;
  test_data->test_class_.attach_thread();

  test_data->ready_count_.fetch_add(1);

  while (test_data->wait_flag_.load()) {};

  bool res;

  int64_t i;
  for (i = thread_id+1; test_data->running_.load(); i += num_threads) {
    int64_t key = (i << 32) + (thread_id << 16);
    int64_t value = key;
    res = test_data->test_class_.insert(key, value);
    assert(res);
  }


  const int64_t max_value = i;

  for (i = thread_id+1; i < max_value; i += num_threads) {
    int64_t temp = -1;
    int64_t key = (i << 32) + (thread_id << 16);
    int64_t value = key;

    res = test_data->test_class_.find(key, temp);
    assert(res && temp == value);
  }

  for (i = thread_id+1; i < max_value; i += num_threads) {
    int64_t key = (i << 32) + (thread_id << 16);
    int64_t temp = key;
    int64_t temp_validate = temp;

    int64_t new_value = ((i+2) << 32) + (thread_id << 16);

    res = test_data->test_class_.update(key, temp, new_value);
    assert(res && temp == temp_validate);
  }

  for (i = thread_id+1; i < max_value; i += num_threads) {
    int64_t key = (i << 32) + (thread_id << 16);
    int64_t temp_validate = ((i+2) << 32) + (thread_id << 16);

    int64_t temp = -1;
    res = test_data->test_class_.find(key, temp);
    assert(res && temp == temp_validate);
  }

  for (i = thread_id+1; i < max_value; i += num_threads) {
    int64_t key = (i << 32) + (thread_id << 16);
    res = test_data->test_class_.remove(key);
    assert(res);
  }

  for (i = thread_id+1; i < max_value; i += num_threads) {
    int64_t key = (i << 32) + (thread_id << 16);
    int64_t temp = -1;
    res = test_data->test_class_.find(key, temp);
    assert(!res && temp == -1);
  }




  test_data->test_class_.detach_thread();
}