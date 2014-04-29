/**
 * This file defines some lightweight structures for manipulating the shared and
 * thread-local information which threads should have access to when running.
 */


#ifndef MEMORY_INFO_H_
#define MEMORY_INFO_H_

#include <atomic>

#include <stdint.h>

#include "tervel/util/memory/hp/hazard_pointer.h"
#include "tervel/util/memory/rc/descriptor_pool.h"
#include "tervel/util/util.h"

namespace tervel {
namespace util {

/**
 * Contains shared information that should be accessible by all threads.
 */
struct SharedInfo {
  //The total number of expected threads in the system.
  uint64_t num_threads;

  //The number of threads which have been assigned an thread_id
  std::atomic<uint64_t> active_threads {0}

  // The shared hazard_pointer object
  HazardPointer *hazard_pointer {nullptr}
};

/**
 * Thread local information. Each thread should have an instance of this.
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
   * has been called, this is incremented at the start of Descriptor::remove
   * and decremented upon return.
   */
  uint64_t recursive_depth {0};

  /** 
   * help_id_ is a variable used to track which thread to check for an
   * announcement.
   *
   * This is used exclusively by the annoucement table function tryHelpAnother*
   *    *(unless function has been renamed)
   */
  uint64_t help_id_ {0}

  /** 
   * delay_count_ is a variable used to delay how often a thread checks for an
   * annoucnement
   * 
   *  This is used exclusively by the annoucement table function tryHelpAnother*
   *    *(unless function has been renamed)
   */
  uint64_t delay_count_ {0}


  // REVIEW(carlos): Careful with adding methods to structs. Rule of thumb is
  //   that structs should be used for plain old data (POD), and classes for
  //   everything else. This is especially important since there is a global
  //   ThreadInfo object, and the hard rule is that you shouldn't use non-POD
  //   objects as globals because the construction and destruction is not
  //   defined.

  /**
   * This function returns the id of the next thread to helper. If the max
   * has been reached it is reset to 0.
   * 
   * @param  max_delay
   * @return current delay count
   */
  inline int help_id(int num_threads) {
    if (help_id_ == num_threads) {
      help_id_ = 0;
    }
    return ++help_id_;
  }

  

  /**
   * This function returns the next delay_count, and if it reaches the limit
   * it is reset to 0.
   * 
   * @param  max_delay
   * @return current delay count
   */
  inline int delay_count(int max_delay) {
    if (delay_count_ == max_delay) {
      delay_count_ = 0;
    }
    return ++delay_count_;
  }

  /**
   * shared_info provides a link to the shared info object. This object contians
   * number of threads, hazard_pointer, and other shared structures.
   */
  SharedInfo *shared_info {nullptr}

  /**
   * This is a link to the threads pool of reference counted descriptor objects.
   */
  memory::rc::DescriptorPool *descriptor_pool {nullptr}
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

class TervelThread {
 public:
    TervelThread() {}
};

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

 private:
  DISALLOW_COPY_AND_ASSIGN(RecursiveAction);
};

}  // namespace tervel
}  // namespace memory

#endif  //  MEMORY_INFO_H_
