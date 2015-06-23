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

#ifndef WF_MCAS_API_H_
#define WF_MCAS_API_H_
#include <string>
#include <functional>

#include <tervel/util/info.h>
#include <tervel/util/tervel.h>
#include <tervel/util/thread_context.h>
#include <tervel/util/memory/hp/hp_list.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/algorithms/wf/mcas/mcas.h>

class TestClass {
 public:
  TestClass(size_t num_threads) {
    tervel_obj = new tervel::Tervel(num_threads);
    attach_thread();
  }

  std::string toString() {
    return "WF MCAS";
  };

  void attach_thread() {
    tervel::ThreadContext* thread_context __attribute__((unused));
    thread_context = new tervel::ThreadContext(tervel_obj);
  };

  void detach_thread() {};

  void * calc_next_value(void * value) {
    uintptr_t temp = reinterpret_cast<uintptr_t>(value);
    temp = (temp + 0x16) & (~3);
    return reinterpret_cast<void *>(temp);
  }

  bool mcas(int len, std::function<int()> posFunc,  std::atomic<void *> *address) {

    tervel::algorithms::wf::mcas::MultiWordCompareAndSwap<void *> *mcas;
    mcas = new tervel::algorithms::wf::mcas::MultiWordCompareAndSwap<void *>(len);

    for (int i = 0; i < len; i++) {
      bool success;
      do {
        int var = posFunc();
        std::atomic<void *> *pos = &(address[var]);
        void * expected_value = tervel::algorithms::wf::mcas::read<void *>(pos);
        void * new_value = calc_next_value(expected_value);
        success = mcas->add_cas_triple(pos, expected_value, new_value);
      }while(!success);
    }

    bool res = mcas->execute();
    mcas->safe_delete();
    return res;
  }


 private:
  tervel::Tervel* tervel_obj;
};

#endif  // WF_MCAS_API_H_
