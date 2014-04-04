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

#include "wf_mcas_3.hpp"

enum TEST_TYPE_CODE {UPDATEOBJECT,
    UPDATEMULTIOBJECT, RANDOMOVERLAPS};

#ifdef TEST_TYPE
  TEST_TYPE_CODE testType= TEST_TYPE;
#else
  TEST_TYPE_CODE testType= RANDOMOVERLAPS;
#endif



struct Results {
  int64 passed_count;
  int64 failed_count;

  void add(struct Results *o) {
    passed_count+=o->passed_count;
    failed_count+=o->failed_count;
  }
};
struct Results * results;

std::atomic<bool> running(true);
std::atomic<bool> waitFlag(true);
std::atomic<int64> *testArray;
std::atomic<bool>* isReady;

const int arrayLength = 64;
const int MWORDS = 4;
const int kThreads = 64;
const int exeTime = 60;

void run(int Thread_ID);
int main(int argc, const char * argv[]) {
  ucf::thread::Initilize_Threading_Manager(kThreads);

  waitFlag.store(true);

  testArray = new std::atomic<int64>[arrayLength];
  for (int i = 0; i < arrayLength; i++) {
    testArray[i].store(0x8);
  }

  results = new struct Results[kThreads];
  std::thread threads[kThreads];
  isReady = new std::atomic<bool>[kThreads];

  for (int i = 0; i < kThreads; i++) {
    isReady[i].store(false);
    threads[i] = std::thread(run, i);
  }
  for (int i = 0; i < kThreads; i++) {
    while (!isReady[i].load()) {}
  }
  waitFlag.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(exeTime));
  running.store(false);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  for (int i = 0; i < kThreads; i++) {
    threads[i].join();
  }

  for (int i = 1; i < kThreads; i++) {
    results[0].add(&results[1]);
  }

  printf("Completed[Passed: %ld, Failed: %ld]\n",
    results[0].passed_count, results[0].failed_count);
  for (int i = 1; i < arrayLength; i++) {
    if (testArray[i].load() != testArray[0].load()) {
      printf("Mismatch(0, %d)::>", i);
      for (int i = 0; i < arrayLength; i++) {
        printf("[%d:%ld] ", i, testArray[i].load());
      }
      printf("\n");
      break;
    }
  }


  ucf::thread::Destory_Threading_Manager();
  return 1;
}


void run_updateMultiObject(int Thread_ID);
void run_RandomOverlaps(int Thread_ID);
void run_updateObject(int Thread_ID, int start_pos);
void run(int Thread_ID) {
  ucf::thread::attachThread();

  switch (testType) {
    case UPDATEOBJECT:
      run_updateObject(Thread_ID, 0);
      break;
    case UPDATEMULTIOBJECT:
      run_updateMultiObject(Thread_ID);
      break;

    case RANDOMOVERLAPS:
      run_RandomOverlaps(Thread_ID);
      break;
    default:
      printf("Error Non Recongized State Test\n");
  }

  ucf::thread::dettachThread();
}



void run_updateObject(int Thread_ID, int start_pos) {
  int failed_count = 0;
  int passed_count = 0;

  ucf::mcas::MCAS<int64 , MWORDS> *vmcas;

  isReady[Thread_ID].store(true);
  while (waitFlag.load()) {}

  while (running.load()) {
    vmcas = new ucf::mcas::MCAS<int64, MWORDS>();

    bool success;
    for (int i = 0; i < MWORDS; i++) {
      int var = start_pos + i;

      std::atomic<int64> *address = (&testArray[var]);

      int64 old_v = ucf::mcas::read<int64>(address);
      int64 new_v = (old_v + 0x16) & (~3);
      success = vmcas->addCASTriple(address, old_v, new_v);
      assert(success);
      success = false;
    }

    success = vmcas->execute();
    if (success) {
      passed_count++;
    } else {
      failed_count++;
    }
  }  // End Execution Loop

  results[Thread_ID].passed_count = passed_count;
  results[Thread_ID].failed_count = failed_count;
  return;
}


void run_updateMultiObject(int Thread_ID) {
  int failed_count = 0;
  int passed_count = 0;

  int max_start_pos=(arrayLength/MWORDS);
  boost::mt19937 rng(Thread_ID);
  boost::uniform_int<> memory_pos_rand(0, max_start_pos-1);

  ucf::mcas::MCAS<int64 , MWORDS> *vmcas;

  isReady[Thread_ID].store(true);
  while (waitFlag.load()) {}

  while (running.load()) {
    vmcas = new ucf::mcas::MCAS<int64 , MWORDS>();
    int start_pos = memory_pos_rand(rng);
    start_pos = start_pos*MWORDS;

    bool success;
    for (int i = 0; i < MWORDS; i++) {
      int var = start_pos + i;

      std::atomic<int64> *address = (&testArray[var]);

      int64 old_v = ucf::mcas::read<int64>(address);
      int64 new_v = (old_v + 0x16) & (~3);
      success = vmcas->addCASTriple(address, old_v, new_v);

      assert(success);
      success = false;
    }

    success = vmcas->execute();

    if (success) {
      passed_count++;
    } else {
      failed_count++;
    }
  }  // End Execution Loop

  results[Thread_ID].passed_count = passed_count;
  results[Thread_ID].failed_count = failed_count;

  return;
}

void run_RandomOverlaps(int Thread_ID) {
  int failed_count = 0;
  int passed_count = 0;

  boost::mt19937 rng(Thread_ID);
  boost::uniform_int<> memory_pos_rand(0, arrayLength);
  ucf::mcas::MCAS<int64 , MWORDS> *vmcas;

  isReady[Thread_ID].store(true);
  while (waitFlag.load()) {}

  while (running.load()) {
    vmcas = new ucf::mcas::MCAS<int64 , MWORDS>();

    bool success;
    for (int i = 0; i < MWORDS; i++) {
    //  printf("MCAS Params: %d : \n",i);
      do {
        int var = memory_pos_rand(rng);
        std::atomic<int64> *address = (&testArray[var]);

        int64 old_v = ucf::mcas::read<int64>(address);
        int64 new_v = (old_v + 0x16) & (~3);
        success = vmcas->addCASTriple(address, old_v, new_v);
      }while(!success);
      success = false;
    }

    success = vmcas->execute();

    if (success) {
      passed_count++;
    } else {
      failed_count++;
    }
  }  // End Execution Loop

  results[Thread_ID].passed_count = passed_count;
  results[Thread_ID].failed_count = failed_count;

  return;
}
