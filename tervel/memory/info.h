/**
 * This file defines some lightweight structures for manipulating the shared and
 * thread-local information which threads should have access to when running.
 */


#ifndef MEMORY_INFO_H_
#define MEMORY_INFO_H_

#include <atomic>

#include <stdint.h>

#include "tervel/memory/hp/hazard_pointer.h"
#include "tervel/util.h"

namespace tervel {
namespace memory {

/**
 * Contains shared information that should be accessible by all threads.
 */
struct SharedInfo {
  uint64_t num_threads;

  // REVIEW(carlos): When answering a todo like this, remove the original todo
  //   and write the response in a reader-agnostic way so that the new comment
  //   serves as documentation.
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
  // REVIEW(carlos): missing a blank line between code and comment. Wrong type
  //   of comment causes recursive_depth field to be commented out by accident.
  /**
   * recursive_depth: used to track the number of times Descriptor::remove
  // has been called, this is incremented at the start of Descriptor::remove
  // and decremented upon return.
  uint64_t recursive_depth {0};

  // REVIEW(carlos): space after star missing
  /**
   *help_id and delay_count are used exclusively by the announcement table
   * function tryHelpAnother (unless function has been renamed)
   * help_id is the thread_id of a thread to check if that thread has an 
   * announcement
   */

  // REVIEW(carlos): Missing documentation comment.
  // REVIEW(carlos): all member variables should be at the bottom of the struct
  //   definition, methods at the top.
  // REVIEW(carlos): Careful with adding methods to structs. Rule of thumb is
  //   that structs should be used for plain old data (POD), and classes for
  //   everything else. This is especially important since there is a global
  //   ThreadInfo object, and the hard rule is that you shouldn't use non-POD
  //   objects as globals because the construction and destruction is not
  //   defined.
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

  // REVIEW(carlos): Missing documentation comment.
  SharedInfo *shared_info {nullptr}
  rc::DescriptorPool *descriptor_pool {nullptr}

};
// REVIEW(carlos): Since we're using ThreadInfo as a thread_local variable, it
//   needs to be plain old data (POD) for the construction and destruction to be
//   well-defined. Would suggest adding a static_assert under the definition of
//   the struct to prevent future changes from removing this trait:
//     static_assert(std::is_pod<ThreadInfo>::value == true, "ThreadInfo is a"
//        "thread_local structure and must be plain-old-data")
//   also #include <type_traits> at the top for std::is_pod.
//   see also: http://stackoverflow.com/questions/146452 for more on POD

// REVIEW(carlos): globals and thread-local variables can't directly go in the
//   header because at link time the compiler will see multiple versions of the
//   same variable (one for each time the header is included by a different
//   compilation unit). Instead, you have to declare said varibales as extern
//   and have a corresponding non-extern declaration in the cc file:
//     bla.h:
//     extern thread_local tl_blarg;
//
//     bla.cc:
//     thread_local tl_blarg;
// REVIEW(carlos): global/thread_local declarations should go at the top of the
//   file. This will require a forward declaration of ThreadInfo, so it's icky,
//   but it's a small price to pay for having all globals in a single place in a
//   file.
thread_local ThreadInfo tl_thread_info;

/**
 * Helper class for RAII management of recursive helping of threads. Lifetime
 * of this object handles the increment and decrement of the `recursive_depth`
 * of the given ThreadInfo object and sets the `recursive_return` if needed.
 */
class RecursiveAction {
 public:
  RecursiveAction() {
    // REVIEW(carlos): Don't bother trying to line up broken lines, just move to
    //   next line and indent 2 indentation levels (4 spaces). Ex:
    //     if (some_long_variable_name >
    //         some_other_long_variable_name) {
    if (tl_thread_info.recursive_depth >
                              tl_thread_info.shared_info->num_threads + 1) {
      tl_thread_info.recursive_return = true;
    }
    tl_thread_info.recursive_depth += 1;
  }

  ~RecursiveAction() { tl_thread_info.recursive_depth -= 1; }

 private:
  DISALLOW_COPY_AND_ASSIGN(RecursiveAction);
};

}  // namespace tervel
}  // namespace memory

#endif  //  MEMORY_INFO_H_
