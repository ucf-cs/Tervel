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
#include <tervel/containers/wf/vector/vector.hpp>
#include <tervel/util/info.h>
#include <tervel/util/thread_context.h>
#include <tervel/util/tervel.h>

typedef uint64_t Value;
typedef tervel::containers::wf::vector::Vector<Value> container_t;


#include "../src/main.h"

DEFINE_int32(capacity, 0, "The initial size of the vector");
DEFINE_int32(prefill, 0, "The number elements to prefill the vector with.");

#define DS_DECLARE_CODE \
  tervel::Tervel* tervel_obj; \
  container_t *container;

#define DS_DESTORY_CODE

#define DS_ATTACH_THREAD \
tervel::ThreadContext* thread_context __attribute__((unused)); \
thread_context = new tervel::ThreadContext(tervel_obj);

#define DS_DETACH_THREAD

#define DS_INIT_CODE \
tervel_obj = new tervel::Tervel(FLAGS_num_threads+1); \
DS_ATTACH_THREAD \
container = new container_t(); \
\
std::default_random_engine generator; \
std::uniform_int_distribution<Value> largeValue(0, UINT_MAX); \
for (int i = 0; i < FLAGS_prefill; i++) { \
  Value x = largeValue(generator) & (~0x3); \
  container->push_back(x); \
}

#define DS_NAME "WF Vector"


#define DS_STATE_STR \
   "\n" _DS_CONFIG_INDENT "size : " + std::to_string(container->size()) + ""

#define DS_CONFIG_STR \
   "\n" _DS_CONFIG_INDENT "Prefill : " + std::to_string(FLAGS_prefill) +"" + \
   "\n" _DS_CONFIG_INDENT "Capacity : " + std::to_string(FLAGS_capacity) +"" + tervel_obj->get_config_str() + DS_STATE_STR


#define OP_RAND \
  std::uniform_int_distribution<Value> random(1, UINT_MAX); \
  /*int ecount = 0; */ \


#define OP_CODE \
  MACRO_OP_MAKER(0, { \
      size_t s = container->size(); \
      if (s == 0) { \
        opRes = false; \
      } else { \
        int idx = random(generator) % s;\
        Value value; \
        opRes = container->at(idx, value); \
      }\
    } \
  ) \
  MACRO_OP_MAKER(1, { \
      size_t s = container->size(); \
      if (s == 0) { \
        opRes = false; \
      } else { \
        int idx = random(generator) % s;\
        Value old_value; \
        container->at(idx, old_value); \
        if (old_value & uint64_t(0x3) != uint64_t(0)) { \
          opRes = false; \
        } else { \
          Value new_value = new_value*2; \
          opRes = container->cas(idx, old_value, new_value); \
        } \
      }\
    } \
  ) \
  MACRO_OP_MAKER(2, { \
      Value temp = reinterpret_cast<Value>(thread_id); \
      temp = temp << (sizeof(Value)*8-7); \
      temp = temp | (lcount << 3); \
      opRes = container->push_back(temp); \
    } \
  ) \
  MACRO_OP_MAKER(3, { \
      Value value = -1; \
      opRes = container->pop_back(value); \
    } \
  ) \
  MACRO_OP_MAKER(4, { \
      container->size(); \
    } \
  ) \
  MACRO_OP_MAKER(5, { \
    size_t s = container->size(); \
    if (s == 0) { \
      opRes = false; \
    } else { \
      int idx = random(generator) % s; \
      Value value = -1; \
      opRes = container->eraseAt(idx, value); \
    } \
  } \
  ) \
  MACRO_OP_MAKER(6, { \
    size_t s = container->size(); \
    if (s == 0) { \
      opRes = false; \
    } else { \
      int idx = random(generator) % s; \
      Value temp = reinterpret_cast<Value>(thread_id); \
      temp = temp << (sizeof(Value)*8-7); \
      temp = temp | (lcount << 3); \
      opRes = container->insertAt(idx, temp); \
    } \
  }\
  ) \

#define DS_OP_NAMES "at", "cas", "pushBack", "popBack", "size", "eraseAt", "insertAt"

#define DS_OP_COUNT 7

inline void sanity_check(container_t *container) {};

#endif  // DS_API_H_
