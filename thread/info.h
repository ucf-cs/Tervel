/**
 * This file defines some lightweight structures for manipulating the shared and
 * thread-local information which threads should have access to when running.
 */


#ifndef THREAD_INFO_H_
#define THREAD_INFO_H_

#include <atomic>

#include <stdint.h>


namespace ucf {
namespace thread {

/**
 * Contains shared information that should be accessible by all threads.
 */
struct SharedInfo {
  uint64_t num_threads;

  // TODO(carlos) what diffrientiates this from the above `num_threads`?
  std::atomic<uint64_t> thread_count {0};
};

/**
 * Thread local information. Each thread should have an instance of this.
 */
struct ThreadInfo {
  /**
   * A unique ID among all active threads.
   */
  uint64_t thread_id;

  // TODO(carlos) what are the below members for?
  bool recursive_return {false};
  uint64_t recursive_depth {0};
  uint64_t help_id {0};
  uint64_t delay_count {0};
};

/**
 * Helper class for RAII management of recursive helping of threads. Lifetime
 * of this object handles the increment and decrement of the `recursive_depth`
 * of the given ThreadInfo object and sets the `recursive_return` if needed.
 */
class RecursiveAction {
 public:
  RecursiveAction(const SharedInfo &shared_info, ThreadInfo *local_info)
      : shared_info_(shared_info), local_info_(local_info) {
    if (local_info_->recursive_depth > shared_info.num_threads + 1) {
      local_info_->recursive_return = true;
    }
    local_info_->recursive_depth += 1;
  }

  ~RecursiveAction() { local_info_->recursive_depth -= 1; }

  const SharedInfo &shared_info_;
  ThreadInfo *local_info_;
};

}  // namespace ucf
}  // namespace thread

#endif  //  THREAD_INFO_H_
