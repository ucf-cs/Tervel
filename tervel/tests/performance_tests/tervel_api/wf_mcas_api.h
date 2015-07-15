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

#ifndef DS_API_H_
#define DS_API_H_


#include <string>
#include <tervel/util/info.h>
#include <tervel/util/thread_context.h>
#include <tervel/util/tervel.h>

#include <tervel/algorithms/wf/mcas/mcas.h>

#include "../src/main.h"

#include <gflags/gflags.h>


#define DS_NAME "WF MCAS"

#define DS_CONFIG_STR \
   "\n" _DS_CONFIG_INDENT "ArrayLength : " + std::to_string(FLAGS_array_length) +"" + \
   "\n" _DS_CONFIG_INDENT "McasSize : " + std::to_string(FLAGS_mcas_size) +"" \
   "\n" _DS_CONFIG_INDENT "isOverlapping : " + std::to_string(FLAGS_overlapping) +"" \
   "\n" _DS_CONFIG_INDENT "isMultipleObjects : " + std::to_string(FLAGS_multipleObjects) +""

#define DS_STATE_STR

// Constructor Arguments
DEFINE_bool(overlapping, false, "Whether or not the mcas operations can be overlapping.");

DEFINE_bool(multipleObjects, false, "Whether or not multiple disjoint mcas operations can occur.");

DEFINE_int32(array_length, 32, "The size of the region to test on.");

DEFINE_int32(mcas_size, 2, "The number of words in a mcas operation.");

typedef int container_t;
typedef void * Value;
typedef tervel::algorithms::wf::mcas::MultiWordCompareAndSwap<Value> containter_t;

struct counter{
  int inc() {
    assert(x_ >= 0 && x_ < limit_);
    int y = x_++;
    y += offset_;
    assert(y >= 0 && y < limit_);
    return y;
  };

  void reset1(int offset, int len) {
    offset_ = offset * len;
    x_ = 0;
  };

  void reset2(std::function<int()> func, int len) {
    offset_ = func() * len;
    x_ = 0;
  };

  void setLimit(int l) {
    limit_ = l;
  };

  int limit_ = 0;
  int x_ = 0;
  int offset_ = 0;
};



#define DS_DECLARE_CODE \
  container_t *container = nullptr; \
  tervel::Tervel* tervel_obj; \
  std::atomic<void *>* shared_memory; \

#define DS_DESTORY_CODE

#define DS_ATTACH_THREAD \
tervel::ThreadContext* thread_context __attribute__((unused)); \
thread_context = new tervel::ThreadContext(tervel_obj);

#define DS_DETACH_THREAD

#define DS_INIT_CODE \
if (FLAGS_overlapping && FLAGS_multipleObjects) { \
  assert(false && "Can not have both multipleObjects and overlapping flags set"); \
  exit(-1); \
} \
tervel_obj = new tervel::Tervel(FLAGS_num_threads+1); \
DS_ATTACH_THREAD \
shared_memory = new std::atomic<void *>[FLAGS_array_length]; \
for (int i = 0; i < FLAGS_array_length; i++) { \
  shared_memory[i].store(0); \
}



#define OP_RAND \
  /* std::uniform_int_distribution<Value> random(1, UINT_MAX); */ \
  std::uniform_int_distribution<int> random_pos(0, FLAGS_array_length); \
  std::uniform_int_distribution<int> multi_objs(0, FLAGS_array_length/FLAGS_mcas_size);

#define OP_CODE \
  MACRO_OP_MAKER(0, \
  {\
    containter_t *mcas = new containter_t(FLAGS_array_length); \
    int offset = 0; \
    if (FLAGS_multipleObjects) {\
      offset = multi_objs(generator); \
      if (offset + FLAGS_mcas_size > FLAGS_array_length) {\
        offset = FLAGS_mcas_size - FLAGS_array_length;\
      }\
    }\
    for (int i = 0; i < FLAGS_mcas_size; i++) { \
      while(true) { \
        int var; \
        if (FLAGS_overlapping) { \
          var = random_pos(generator);\
        } else { \
          var = i + offset; \
        }\
        std::atomic<Value> *pos = &(shared_memory[var]); \
        Value cur = tervel::algorithms::wf::mcas::read<Value>(pos); \
        Value next = (Value)(((uintptr_t)cur + 0x16) & (~3)); \
        if (mcas->add_cas_triple(pos, cur, next)) { \
          break; \
        }\
      }\
    } \
    opRes = mcas->execute(); \
    mcas->safe_delete(); \
  }\
  )\
  MACRO_OP_MAKER(1, \
  {\
    int var = random_pos(generator);\
    std::atomic<Value> *pos = &(shared_memory[var]); \
    tervel::algorithms::wf::mcas::read<Value>(pos); \
  }\
  )\

#define DS_OP_NAMES "mcas", "read"

#define DS_OP_COUNT 2

inline void sanity_check(container_t *container) {};

#endif  // DS_API_H_
