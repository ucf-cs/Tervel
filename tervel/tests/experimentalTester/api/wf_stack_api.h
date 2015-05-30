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

#ifndef WF_STACK_API_H_
#define WF_STACK_API_H_


#include <string>
#include <tervel/containers/wf/stack/stack.h>
#include <tervel/util/info.h>
#include <tervel/util/thread_context.h>
#include <tervel/util/tervel.h>

typedef int64_t Value;
typedef tervel::containers::wf::Stack<Value> container_t;


#include "../testerMacros.h"

DEFINE_int32(prefill, 0, "The number elements to place in the stack on init.");

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
sanity_check(container); \
\
std::default_random_engine generator; \
std::uniform_int_distribution<Value> largeValue(0, UINT_MAX); \
for (int i = 0; i < FLAGS_prefill; i++) { \
  Value x = largeValue(generator) & (~0x3); \
  container->push(x); \
}

#define DS_NAME "WF Stack"

#define DS_CONFIG_STR \
   "\n  Prefill : " + std::to_string(FLAGS_prefill) +""

#define OP_RAND \
  /* std::uniform_int_distribution<Value> random(1, UINT_MAX); */ \
  int ecount = 0;


#define OP_CODE \
 MACRO_OP_MAKER(0, { \
      Value value; \
      opRes = container->pop(value); \
    } \
  ) \
 MACRO_OP_MAKER(1, { \
      /* Value value = random(); */ \
      Value value = (thread_id << 56) | ecount; \
      opRes = container->push(value); \
    } \
  )

#define OP_NAMES "pop", "push"

#define DS_OP_COUNT 2

inline void sanity_check(container_t *stack) {
  bool res;
  Value i, j, temp;

  int limit = 100;

  for (i = 0; i < limit; i++) {
    bool res = stack->push(i);
    assert(res && "If this assert fails then the there is an issue with either pushing or determining that it is full");
  };
  i--;

  for (j = 0; j < limit; j++) {
    res = stack->pop(temp);
    assert(res && "If this assert fails then the there is an issue with either poping or determining that it is not empty");
    assert(temp==i && "If this assert fails then there is an issue with  determining the pop element");
    i--;
  };

  res = stack->pop(temp);
  assert(!res && "If this assert fails then there is an issue with pop or determining that it is empty");
};

#endif  // WF_STACK_API_H_
