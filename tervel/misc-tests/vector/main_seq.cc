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

#include <thread>
#include <time.h>
#include <vector>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <gflags/gflags.h>

#include <tervel/containers/wf/vector/vector.hpp>
#include <tervel/util/info.h>
#include <tervel/util/thread_context.h>
#include <tervel/util/tervel.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hp_list.h>

tervel::containers::wf::vector::Vector<long> *container;

void print_vector();

int main(int argc, char **argv) {
  int capacity = 32;
  int num_threads = 1;

  tervel::Tervel *tervel_obj = new tervel::Tervel(num_threads);
  tervel::ThreadContext* thread_context __attribute__((unused));
  thread_context = new tervel::ThreadContext(tervel_obj);

  container = new tervel::containers::wf::vector::Vector<long>(capacity);

  long temp = 0x8;
  long element_count = 1;
  container->push_back(temp);
  for(int i = 0; i < 15; i++) {
    temp += 0x8;
    container->insertAt(0,temp);
    element_count++;
  }
  print_vector();

  for(int i = 0; i < element_count; i++) {
    long temp2 = -1;
    bool res = container->eraseAt(0,temp2);
    assert(res);
    assert(temp2 == temp);
    temp -= 0x8;
  }
  print_vector();

  {
    long temp2 = -1;
    bool res = container->eraseAt(0,temp2);
    assert(res == false);
    assert(temp2 == -1);
  }

  print_vector();
  return 0;
}

void print_vector() {

  for(int i = 0; i < 18; i+=1) {
    long temp = -1;
    container->at(i, temp);
    std::cout << i << " : " << temp << std::endl;
  }
  std::cout << "====" << std::endl;
}