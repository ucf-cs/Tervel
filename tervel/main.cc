//
//  main_tester.c
//  mCAS
//
//  Created by Steven Feldman on 11/1/12.
//  Copyright (c) 2012 Steven FELDMAN. All rights reserved.
//

// #define KILL_THREAD 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <thread>

#include <iostream>
#include <atomic>

#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>
#include <gflags/gflags.h>

#include "tervel/mcas/mcas.h"
#include "tervel/util/info.h"

extern thread_local ThreadContext * tl_thread_info;

enum class TestType {UPDATEOBJECT, UPDATEMULTIOBJECT, RANDOMOVERLAPS};

struct Results {
  std::atomic<uint64_t> passed_count_ {0}
  std::atomic<uint64_t> failed_count_ {0}

  void atomic_add(int passed_count, int failed_count) {
    passed_count_.fetch_add(passed_count);
    failed_count_.fetch_add(failed_count);
  }
};
struct Results test_results;

std::atomic<bool> running {true}
std::atomic<bool> wait_flag {true}
std::atomic<uint64_t>[] shared_memory;
std::atomic<int> ready_count {0}



void run(int thread_id, TestType operation_type);

DEFINE_int(num_threads, 1, "The number of threads to spawn.");
DEFINE_int(execution_time, 5, "The amount of time to run the tests");
DEFINE_int(array_length, 64, "The size of the region to test on.");
DEFINE_int(mcas_size, 2, "The number of words in a mcas operation.");
DEFINE_int(test_type, 0, "The type of test to execute"
                          + "(0: updating sinlge multi-word object"
                          + ", 1: updating multiple objects"
                          + ", 2: updating overlapping mult-word updates)");

int main(int argc, const char * argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  const int num_threads = FLAG_num_threads;
  const int execution_time = FLAG_execution_time;
  const int array_length = FLAG_array_length;
  const int mcas_size = FLAG_mcas_size;
  const TestType operation_type =  FLAG_operation_type;

  tervel::Initilize_Tervel(num_threads);

  shared_memory = new std::atomic<uint64_t>[arrayLength](0x8);

  std::thread threads[num_threads];
  for (int i = 0; i < num_threads; i++) {
    threads[i] = std::thread(run, i, operation_type);
  }

  while (ready_count.load() < num_threads) {}
  
  wait_flag.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(exeTime));
  running.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  for (int i = 0; i < num_threads; i++) {
    threads[i].join();
  }

  for (int i = 1; i < num_threads; i++) {
    results[0].add(&results[1]);
  }

  printf("Completed[Passed: %ld, Failed: %ld]\n",
    results[0].passed_count, results[0].failed_count);
  for (int i = 1; i < arrayLength; i++) {
    if (shared_memory[i].load() != shared_memory[0].load()) {
      printf("Mismatch(0, %d)::>", i);
      for (int i = 0; i < arrayLength; i++) {
        printf("[%d:%ld] ", i, shared_memory[i].load());
      }
      printf("\n");
      break;
    }
  }


  tervel::Destroy_Tervel();
  return 1;
}


void run_update_multible_objects(int thread_id);
void run_RandomOverlaps(int thread_id);
void run_update_object(int thread_id, int start_pos);

void run(int thread_id, TestType operation_type) {
  tervel::attachThread();

  switch (operation_type) {
    case UPDATEOBJECT:
      run_update_object(thread_id, 0);
      break;

    case UPDATEMULTIOBJECT:
      run_update_multible_objects(thread_id);
      break;

    case RANDOMOVERLAPS:
      run_RandomOverlaps(thread_id);
      break;

    default:
      printf("Error Non Recongized State Test\n");
  }

  tervel::dettachThread();
}



void run_update_object(int thread_id, int start_pos) {
  int failed_count = 0;
  int passed_count = 0;

  tervel::mcas::MCAS<uint64_t> *mcas;

  ready_count.fetch_add(1);
  while (wait_flag.load()) {}

  while (running.load()) {
    mcas = new tervel::mcas::MCAS<uint64_t>();

    for (int i = 0; i < MWORDS; i++) {
      int var = start_pos + i;

      std::atomic<uint64_t> *address = (&shared_memory[var]);
      uint64_t expected_value = tervel::Descriptor::read(address);
      uint64_t new_value = (expected_value + 0x16) & (~3);

      bool success = mcas->add_CAS_triple(address, expected_value, new_value);
      assert(success);
    }

    if (mcas->execute()) {
      passed_count++;
    } else {
      failed_count++;
    }
    mcas->safe_delete();
  }  // End Execution Loop

  test_results.atomic_add(passed_count, failed_count);
  return;
}


void run_update_multible_objects(int thread_id) {
  int failed_count = 0;
  int passed_count = 0;

  int max_start_pos=(arrayLength/MWORDS);
  boost::mt19937 rng(thread_id);
  boost::uniform_int<> memory_pos_rand(0, max_start_pos-1);

  tervel::mcas::MCAS<uint64_t> *mcas;

  ready_count.fetch_and_add(1);
  while (wait_flag.load()) {}

  while (running.load()) {
    mcas = new tervel::mcas::MCAS<uint64_t>();
    int start_pos = memory_pos_rand(rng) * MWORDS;

    for (int i = 0; i < MWORDS; i++) {
      int var = start_pos + i;

      std::atomic<uint64_t> *address = (&shared_memory[var]);
      uint64_t expected_value = tervel::Descriptor::read(address);
      uint64_t new_value = (expected_value + 0x16) & (~3);

      bool success = mcas->add_CAS_triple(address, expected_value, new_value);
      assert(success);
    }

    if (mcas->execute()) {
      passed_count++;
    } else {
      failed_count++;
    }

    mcas->safe_delete();
  }  // End Execution Loop

  test_results.atomic_add(passed_count, failed_count);
  return;
}

void run_RandomOverlaps(int thread_id) {
  int failed_count = 0;
  int passed_count = 0;

  boost::mt19937 rng(thread_id);
  boost::uniform_int<> memory_pos_rand(0, arrayLength);
  tervel::mcas::MCAS<uint64_t> *mcas;

  ready_count.fetch_and_add(1);
  while (wait_flag.load()) {}

  while (running.load()) {
    mcas = new tervel::mcas::MCAS<uint64_t>();

    bool success;
    for (int i = 0; i < MWORDS; i++) {
      do {
        int var = memory_pos_rand(rng);
        std::atomic<uint64_t> *address = (&shared_memory[var]);
        uint64_t expected_value = tervel::Descriptor::read(address);
        uint64_t new_value = (expected_value + 0x16) & (~3);

        success = mcas->add_CAS_triple(address, expected_value, new_value);
      }while(!success);
    }

    if (mcas->execute()) {
      passed_count++;
    } else {
      failed_count++;
    }
    mcas->safe_delete();
  }  // End Execution Loop

  test_results.atomic_add(passed_count, failed_count);
  return;
}
