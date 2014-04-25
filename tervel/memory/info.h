/**
 * This file defines some lightweight structures for manipulating the shared and
 * thread-local information which threads should have access to when running.
 */


#ifndef MEMORY_INFO_H_
#define MEMORY_INFO_H_

#include <atomic>

#include <stdint.h>
#include "tervel/memory/hp/hazard_pointer.h"

namespace tervel {
namespace memory {

/**
 * Contains shared information that should be accessible by all threads.
 */
struct SharedInfo {
  uint64_t num_threads;

  // TODO(carlos) what diffrientiates this from the above `num_threads`?
  // @Carlos, num_threads is the max unique threads, while thread_count
  // (name could be changed), is used to assign thread ids.
  // In the future, we may wish to change this so that thread ids can be
  // safely returned. For now this is not a priority. -Sven (Sven=Steven)
  std::atomic<uint64_t> thread_count {0}

  HazardPointer *hazard_pointer {nullptr}
};

/**
 * Thread local information. Each thread should have an instance of this.
 *
 * TODO(carlos) figure out what data members are used by what code, and split
 *   this struct into several classes if needed. I think the help_id and
 *   delay_count are used exclusively by ProgressAssurance.
 */
struct ThreadInfo {
  /**
   * A unique ID among all active threads.
   */
  uint64_t thread_id;

  /** 
   * Recurive_return: Used to indicate a thread must return to its own 
   * operation and re-evaualte its state. This is set to true in the event
   * 1) The thread reasons that the dependncy between the current op it is
   * Trying to help has changed and a result it must re-examine its op
   * 2) Max Fail count has been reached and it needs to make an announcement
   * For its operation
   */
  bool recursive_return {false}
  /**
   * recursive_depth: used to track the number of times Descriptor::remove
  // has been called, this is incremented at the start of Descriptor::remove
  // and decremented upon return.
  uint64_t recursive_depth {0};

  /**
   *help_id and delay_count are used exclusively by the announcement table
   * function tryHelpAnother (unless function has been renamed)
   * help_id is the thread_id of a thread to check if that thread has an 
   * announcement
   */

  uint64_t help_id_ {0}
  int help_id(int num_threads) {
    if (help_id_ == num_threads) {
      help_id_ = 0;
    }
    return ++help_id_;
  }

  /** 
   * delay_count is a variable used to delay how often a thread checks for an
   * annoucnement
   */
  uint64_t delay_count_ {0}
  int delay_count(int max_delay) {
    if (delay_count_ == max_delay) {
      delay_count_ = 0;
    }
    return ++delay_count_;
  }

  SharedInfo *shared_info {nullptr}
  rc::DescriptorPool *descriptor_pool {nullptr}

};

thread_local ThreadInfo tl_thread_info;

/**
 * Helper class for RAII management of recursive helping of threads. Lifetime
 * of this object handles the increment and decrement of the `recursive_depth`
 * of the given ThreadInfo object and sets the `recursive_return` if needed.
 */
class RecursiveAction {
 public:
  RecursiveAction() {
    if (tl_thread_info.recursive_depth >
                              tl_thread_info.shared_info->num_threads + 1) {
      tl_thread_info.recursive_return = true;
    }
    tl_thread_info.recursive_depth += 1;
  }

  ~RecursiveAction() { tl_thread_info.recursive_depth -= 1; }
};

}  // namespace tervel
}  // namespace memory

#endif  //  MEMORY_INFO_H_
