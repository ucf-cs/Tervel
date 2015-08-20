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
#include <tervel/containers/lf/linked_list_queue/queue.h>

typedef int64_t Value;
typedef tervel::containers::lf::Queue<Value> container_t;


#include "../src/main.h"

DEFINE_int32(prefill, 0, "The number elements to place in the container on init.");


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
std::default_random_engine generator; \
std::uniform_int_distribution<Value> largeValue(0, UINT_MAX); \
for (int i = 0; i < FLAGS_prefill; i++) { \
  Value x = largeValue(generator) & (~0x3); \
  container->enqueue(x); \
}

#define DS_NAME "LF LinkedList Queue"

#define DS_CONFIG_STR \
   "\n" _DS_CONFIG_INDENT "Prefill : " + std::to_string(FLAGS_prefill) + "" + tervel_obj->get_config_str() + ""

#define DS_STATE_STR \
   "\n" _DS_CONFIG_INDENT "size : " + std::to_string(container->size()) + ""

#define OP_RAND \
  /* std::uniform_int_distribution<Value> random(1, UINT_MAX); */ \
  int ecount = 0;

/*
  TODO: Remove the need for the numerical argument
  Make adding the function name and code the same step
  DS_OP_COUNT should be set to the last passed MACRO OP
 */
#define OP_CODE \
MACRO_OP_MAKER(0, { \
    container_t::Accessor access; \
    opRes = container->dequeue(access); \
  } \
) \
MACRO_OP_MAKER(1, { \
    /* Value value = random(); */ \
    Value value = (thread_id << 56) | ecount; \
    opRes = container->enqueue(value); \
  } \
) \
MACRO_OP_MAKER(2, { \
    container->empty(); \
  } \
) \
MACRO_OP_MAKER(3, { \
    container->size(); \
  } \
)

#define DS_OP_NAMES "dequeue", "enqueue", "empty", "size"

#define DS_OP_COUNT 4


inline void sanity_check(container_t *container) {
  bool res;
  Value i, j, temp;

  int limit = 100;

  for (i = 0; i < limit; i++) {
    bool res = container->enqueue(i);
    assert(res && "If this assert fails then the there is an issue with either pushing or determining that it is full");
    assert(container->size() == i + 1 && "If this assert fails then the there is an issue with the size counter");
  };

  for (j = 0; j < limit; j++) {
    container_t::Accessor access;
    res = container->dequeue(access);
    assert(res && "If this assert fails then the there is an issue with either popping or determining that it is not empty");
    temp = access.value();
    assert(temp==j && "If this assert fails then there is an issue with  determining the pop element");

    i--;
    assert(container->size() == i  && "If this assert fails then the there is an issue with the size counter");

  };

  {
    container_t::Accessor access;
    res = container->dequeue(access);
    assert(!res && "If this assert fails then there is an issue with pop or determining that it is empty");
  }
};
#endif  // DS_API_H_
