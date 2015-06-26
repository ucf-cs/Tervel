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
#ifndef __TESTER_YAML_OUTPUT_H_
#define __TESTER_YAML_OUTPUT_H_


std::string config_str(int numThreads, std::string execution_str) {
  std::string res("alg_config:\n");
  res += "  algorithm_name : " DS_NAME "\n";
  res += "  ds_config : " DS_CONFIG_STR "\n";
  res += "  run_config : " + execution_str + "\n";

  res += "test_config :  \n";
  res += "  execution_time : " + std::to_string(FLAGS_execution_time) + "\n";
  res += "  main_delay : " + std::to_string(FLAGS_main_sleep) + "\n";
  res += "  num_threads : " + std::to_string(numThreads) + "\n";

  return res;
};

std::string results_str(double start_time, double end_time, int numThreads) {
  std::string res("");
  res += "time_stamps: \n";
  res += "  start : " + std::to_string(start_time) + "\n";
  res += "  end : " + std::to_string(end_time) + "\n";

  res += "metrics : \n";
  res += "  operations : \n";
  res += "    totals : \n";
  for (int j = 0; j < DS_OP_COUNT; j++) {
    int p = 0; int f = 0;
    for (int i = 0; i < numThreads; i++) {
      p += g_test_results[i][j].pass();
      f += g_test_results[i][j].fail();
    }

    res += "      " + op_names[j] + "_Pass : "
          + std::to_string(p) + "\n";
    res += "      " + op_names[j] + "_Fail : "
          + std::to_string(f) + "\n";
  }

  res += "    per_thread : \n";
  for (int i = 0; i < numThreads; i++) {
    res += "      - TID : "  + std::to_string(i) + "\n";
    for (int j = 0; j < DS_OP_COUNT; j++) {
      int p = g_test_results[i][j].pass();
      int f = g_test_results[i][j].fail();
      float r = g_test_results[i][j].rate();

      res += "        " +  op_names[j] + "_Pass : "
            + std::to_string(p) + "\n";
      res += "        " +  op_names[j] + "_Fail : "
            + std::to_string(f) + "\n";
      res += "        " +  op_names[j] + "_Rate : "
          + std::to_string(r) + "\n";
    }
  }

  return res;
};


#endif  // #ifndef __TESTER_YAML_OUTPUT_H_