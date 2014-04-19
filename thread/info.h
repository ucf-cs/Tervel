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

}  // namespace ucf
}  // namespace thread

#endif  //  THREAD_INFO_H_
