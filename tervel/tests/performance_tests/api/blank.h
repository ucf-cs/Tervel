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

#include <atomic>
#include "../src/main.h"

DEFINE_int64(value, 0, "The initial value of the counter");


#define DS_DECLARE_CODE \
  std::atomic<int64_t> val;

#define DS_DESTORY_CODE

#define DS_ATTACH_THREAD

#define DS_DETACH_THREAD

#define DS_INIT_CODE \
  val.store(FLAGS_value);

#define DS_NAME "Atomic Int"

#define DS_CONFIG_STR \
    "\n" _DS_CONFIG_INDENT "Value : " + std::to_string(FLAGS_value) + ""


#define OP_RAND \
  std::uniform_int_distribution<int64_t> random(SHRT_MIN, SHRT_MAX);


#define OP_CODE \
  MACRO_OP_MAKER(0, { \
    int64_t value = random(generator); \
    val.store(value); \
  } \
  ) \
  MACRO_OP_MAKER(1, { \
    int64_t value = random(generator); \
    val.fetch_add(value); \
  } \
  ) \
  MACRO_OP_MAKER(2, { \
    int64_t value = random(generator); \
    val.fetch_sub(value); \
  } \
  ) \
  MACRO_OP_MAKER(3, { \
    int64_t value = random(generator); \
    val.exchange(value); \
  } \
  ) \


#define DS_OP_NAMES "store", "faa", "fas", "exchange"

#define DS_OP_COUNT 4

#endif  // DS_API_H_
