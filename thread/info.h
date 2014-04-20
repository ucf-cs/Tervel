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
  // @Carlos, num_threads is the max unique threads, while thread_count 
  // (name could be changed), is used to assign thread ids.
  // In the future, we may wish to change this so that thread ids can be 
  // safely returned. For now this is not a priority. -Sven (Sven=Steven)
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
  // Recurive_return: Used to indicate a thread must return to its own 
  // operation and re-evaualte its state. This is set to true in the event
  // 1) The thread reasons that the dependncy between the current op it is
  // Trying to help has changed and a result it must re-examine its op
  // 2) Max Fail count has been reached and it needs to make an announcement
  // For its operation
  bool recursive_return {false};
  // recursive_depth: used to track the number of times Descriptor::remove
  // has been called, this is incremented at the start of Descriptor::remove
  // and decremented upon return.
  uint64_t recursive_depth {0};

  //help_id and delay_count are used exclusively by the announcement table
  // function tryHelpAnother (unless function has been renamed)
  // help_id is the thread_id of a thread to check if that thread has an 
  // announcement
  // delay_count is a variable used to delay how often a thread checks for an
  // annoucnement
  uint64_t help_id {0};
  uint64_t delay_count {0};
};

/**
 * Helper class for RAII management of recursive helping of threads. Lifetime
 * of this object handles the increment and decrement of the `recursive_depth`
 * of the given ThreadInfo object and sets the `recursive_return` if needed.
 * TODO(Carlos), this just feels bloated to me for a number of reasons.
 * 1)The parameters on the construction, these are thread local/global constants
 * Why do we need to pass them in, when we can access the source from this scope
 * 2) Why do we need a class which increments/decrements, why not just call the
 * the correct functions. These are used in one function, with no foreseeable
 * use elsewhere. We save one line of code in the other function, but force the
 * person reading it to understand this function. It also adds unto the stack
 * unnecessary the initlization of this class and descrution.
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
