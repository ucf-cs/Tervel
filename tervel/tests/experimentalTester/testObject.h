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

typedef struct{
  std::atomic<bool> wait_;
  std::atomic<bool> execute_;
  std::atomic<uint64_t> ready_count_;
  std::atomic<uint64_t> finished_count_;

  void init() {
    wait_.store(true);
    execute_.store(true);
    ready_count_.store(0);
    finished_count_.store(0);
  }
  void start() {
    wait_.store(false);
  }
  void stop() {
    execute_.store(false);
  }
  bool wait() { return wait_.load(); };
  bool execute() { return execute_.load(); };

  bool notReady(uint64_t threads) {
    return ready_count_.load() < threads;
  };

  bool notFinished(uint64_t threads) {
    return finished_count_.load() < threads;
  };
  void ready() { ready_count_.fetch_add(1); };
  void finished() { finished_count_.fetch_add(1); };



} ThreadSignal;
typedef struct{
  int fail_;
  int pass_;
  float rate_;
  void init(float rate) {
    fail_ = 0;
    pass_ = 0;
    rate_ = rate;
  };

  void inc(bool res) {
    if (res) {
      pass_++;
    } else {
      fail_ ++;
    }
  };

  int fail() {
    return fail_;
  };

  int pass() {
    return pass_;
  };

  float rate() {
    return rate_;
  };

}op_counter_t;

class TestObject {
 public:
  TestObject(int argc, char **argv)
      : num_threads_(FLAGS_num_threads)
      , execution_time_(FLAGS_execution_time) {

        test_results_ = new op_counter_t *[num_threads_];

        execution_str_ = "";
        for (int i = 1; i < argc; i++) {
          execution_str_ += std::to_string(atoi(argv[i])) + " ";
        }
      };

  ~TestObject() {

  };

  void set_start_time(double t) {
    this->start_time = t;
  }
  void set_end_time(double t) {
    this->end_time = t;
  }


  void extra_end_signal() {
    /* This function is useful for enabling a blocking operation to return*/
  };

  void init() {
    DS_INIT_CODE
  }
  void destroy() {
    DS_DESTORY_CODE
  };

  void run(int64_t thread_id, char **argv) {
    // Initial Setup
    DS_ATTACH_THREAD

    op_counter_t *op_counter = new op_counter_t[DS_OP_COUNT];

    int lcount = 0;

    int func_call_rate[DS_OP_COUNT];
    int max_rand = 0;
    for (int i = 0; i < DS_OP_COUNT; i++) {
      int rate = atoi(argv[i]);
      op_counter[i].init(rate);
      func_call_rate[i] = rate + max_rand;
      max_rand = func_call_rate[i];
    }
    func_call_rate[DS_OP_COUNT-1]++;

    for (int i = 0; i < DS_OP_COUNT; i++) {
      op_counter[i].rate_ /= (float)(max_rand);
    }

    // Setup Random Number Generation
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(1, max_rand+1);

    OP_RAND

    // Wait for start signal
    ready_count_.fetch_add(1);
    while (wait_flag_.load());

    /** Update this when adding a new data structure **/
    while (running_.load()) {
      lcount++;
      int op = distribution(generator);

      bool opRes;

      OP_CODE

      assert(false);
    }
    ready_count_.fetch_add(1);

    test_results_[thread_id] = op_counter;

    DS_DETACH_THREAD
  };

  std::string yaml_results(int numThreads) {
    std::string res("");
    res += "AlgorithmName : " DS_NAME "\n";
    res += "AlgorithmConfig : " DS_CONFIG_STR "\n";
    res += "ExecutionTime : " + std::to_string(execution_time_) + "\n";
    res += "MainDelay : " + std::to_string(FLAGS_main_sleep) + "\n";
    res += "NumberThreads : " + std::to_string(numThreads) + "\n";
    res += "RunConfig : " + execution_str_ + "\n";

    res += "Time : \n";
    res += "  start : " + std::to_string(start_time) + "\n";
    res += "  end : " + std::to_string(end_time) + "\n";

    res += "Totals : \n";
    for (int j = 0; j < DS_OP_COUNT; j++) {
      int p = 0; int f = 0;
      for (int i = 0; i < numThreads; i++) {
        p += test_results_[i][j].pass();
        f += test_results_[i][j].fail();
      }

      res += "  " + op_names[j] + "_Pass : "
            + std::to_string(p) + "\n";
      res += "  " + op_names[j] + "_Fail : "
            + std::to_string(f) + "\n";
    }

    res += "Threads : \n";
    for (int i = 0; i < numThreads; i++) {
      res += "  - TID : "  + std::to_string(i) + "\n";
      for (int j = 0; j < DS_OP_COUNT; j++) {
        int p = test_results_[i][j].pass();
        int f = test_results_[i][j].fail();
        float r = test_results_[i][j].rate();

        res += "    " + op_names[j] + "_Pass : "
              + std::to_string(p) + "\n";
        res += "    " + op_names[j] + "_Fail : "
              + std::to_string(f) + "\n";
        res += "    " + op_names[j] + "_Rate : "
            + std::to_string(r) + "\n";
      }
    }

    return res;
  }



  std::string results(int numThreads) {
    return yaml_results(numThreads);
  }


  const int num_threads_;
  const int execution_time_;

  DS_DECLARE_CODE

  std::atomic<bool> wait_flag_{true};
  std::atomic<bool> running_{true};
  std::atomic<int> ready_count_{0};

  op_counter_t **test_results_;
  std::string execution_str_;
  double start_time = 0;
  double end_time = 0;
  const std::string op_names[DS_OP_COUNT] = DS_OP_NAMES;
};


