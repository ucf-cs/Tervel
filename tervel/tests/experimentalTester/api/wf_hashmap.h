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
#include <tervel/containers/wf/hash-map/wf_hash_map.h>
#include <tervel/util/info.h>
#include <tervel/util/thread_context.h>
#include <tervel/util/tervel.h>

typedef int64_t Value;
typedef int64_t Key;
typedef typename tervel::containers::wf::HashMap<Key, Value> container_t;
typedef typename container_t::ValueAccessor Accessor;


#include "../testerMacros.h"

DEFINE_int32(prefill, 0, "The number elements to place in the data structure on init.");
DEFINE_int32(capacity, 32768, "The initial capacity of the hashmap, should be a power of two.");
DEFINE_int32(expansion_factor, 5, "The size by which the hash map expands on collision. 2^x = positions, where x is the specified value.");


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
container = new container_t(FLAGS_capacity, FLAGS_expansion_factor); \
\
std::default_random_engine generator; \
std::uniform_int_distribution<Value> largeValue(0, UINT_MAX); \
for (int i = 0; i < FLAGS_prefill; i++) { \
  Value x = largeValue(generator) & (~0x3); \
  container->insert(x, x); \
}

#define DS_NAME "WF Hash Map"

#define DS_CONFIG_STR \
  "Prefill : " + std::to_string(FLAGS_prefill) \
  + "\n  Capacity : " + std::to_string(FLAGS_capacity) \
  + "\n  ExpansionFactor : " + std::to_string(FLAGS_expansion_factor) + ""


#define OP_RAND \
  std::uniform_int_distribution<Value> random(1, USHRT_MAX);


#define OP_CODE \
  MACRO_OP_MAKER(0, { \
    Accessor va; \
    Value value = random(generator); \
    opRes = container->at(value, va); \
    if (opRes) { \
      value = *(va.value()); \
    } \
  } \
  ) \
  MACRO_OP_MAKER(1, { \
    Value value = random(generator); \
    opRes = container->insert(value, value); \
  } \
  ) \
  MACRO_OP_MAKER(2, { \
    Accessor va; \
    Value value = random(generator); \
    opRes = container->at(value, va); \
    if (opRes) { \
      std::atomic<Value> *temp = reinterpret_cast<std::atomic<Value> *>(va.value()); \
      value = *(va.value()); \
      opRes = temp->compare_exchange_strong(value, value * 2); \
    } \
  } \
  ) \
  MACRO_OP_MAKER(3, { \
    Value value = random(generator); \
    opRes = container->remove(value); \
  } \
  ) \


#define OP_NAMES "find", "insert", "update", "delete"

#define DS_OP_COUNT 4

#endif  // DS_API_H_
