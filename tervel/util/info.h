/**
 * This file defines some lightweight structures for manipulating the shared and
 * thread-local information which threads should have access to when running.
 */
#ifndef TERVEL_UTIL_INFO_H_
#define TERVEL_UTIL_INFO_H_

#include <atomic>

#include <stdint.h>


#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/hp/hp_list.h"
#include "tervel/util/memory/hp/list_manager.h"
#include "tervel/util/memory/hp/hazard_pointer.h"
#include "tervel/util/memory/rc/descriptor_pool.h"
#include "tervel/util/memory/rc/pool_manager.h"
#include "tervel/util/util.h"


namespace tervel {

namespace util {
    class RecursiveAction;
}
/**
 * Contains shared information that should be accessible by all threads.
 */
class Tervel{
 public:
  Tervel(int num_threads)
      : num_threads_  {num_threads} 
      , hazard_pointer_(num_threads_)
      , hp_list_manager_(num_threads_)
      , rc_pool_manager_(num_threads_)
      , progress_assurance_(num_threads_) {}
  
  ~Tervel() {
    // TODO implement
  }

 private:
  uint64_t get_thread_id() {
    return active_threads_.fetch_and_add(1);
  }

  // The total number of expected threads in the system.
  const uint64_t num_threads;

  // The number of threads which have been assigned an thread_id
  std::atomic<uint64_t> active_threads_ {0};

  // The shared hazard_pointer object
  const util::memory::hp::HazardPointer hazard_pointer_;

  //Shared HP Element list manager
  const util::memory::hp::ListManager hp_list_manager_;

  //Shared RC Descriptor Pool Manager
  const util::memory::rc::PoolManager rc_pool_manager_;

  //Shared Progress Assurance Object
  const util::ProgressAssurance progress_assurance_;

};

/**
 * Thread local information. Each thread should have an instance of this.
 */
class ThreadContext{
 public:
  ThreadContext(Tervel* tervel)
      : thread_id_ {tervel->get_thread_id()}
      , tervel_ {tervel}
      , rc_descriptor_pool_(tervel_->rc_pool_manager_)
      , hp_element_list_(tervel_->hp_list_manager_) {}

  
  ~ThreadContext() {
    // TODO(steven) delete descriptor pools, return thread id
  }

  util::memory::hp::HazardPointer get_hazard_pointer() {
    return &(tervel_->hazard_pointer_);
  }

  util::ProgressAssurance get_progress_assurance() {
    return &(tervel_->progress_assurance_);
  }



  /**
   * This function returns the id of the next thread to helper. If the max
   * has been reached it is reset to 0.
   * 
   * @param  max_delay
   * @return current delay count
   */
  int help_id(int num_threads) {
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
  int delay_count(int max_delay) {
    if (delay_count_ == max_delay) {
      delay_count_ = 0;
    }
    return ++delay_count_;
  }

  /**
   * @return whether or not the thread is performing a recrusive return.
   */
  bool recursive_return() {
    return recursive_return_;
  }

  /**
   * Sets recrusive_return_ to true
   */
  void set_recursive_return() {
    recursive_return_ = true;
  }

  /**
   * Sets recrusive_return_ to false
   */
  void clear_recursive_return() {
    recursive_return_ = false;
  }

  /**
   * @return the current recursive depth
   */
  size_t get_recursive_depth() {
    return recursive_depth_;
  }

  /**
   * @return the threads id.
   */
  uint64_t get_thread_id() {
    return thread_id_;
  }

  /**
   * @return number of threads
   */
  uint64_t get_num_threads() {
    return tervel_->num_threads_;
  }

 private:
   /**
   * A unique ID among all active threads.
   */
  const uint64_t thread_id_;

  /** 
   * Recurive_return: Used to indicate a thread must return to its own 
   * operation and re-evaualte its state. This is set to true in the event
   * 1) The thread reasons that the dependncy between the current op it is
   * Trying to help has changed and a result it must re-examine its op
   * 2) Max Fail count has been reached and it needs to make an announcement
   * For its operation
   */
  bool recursive_return_ {false};

  /**
   * recursive_depth: used to track the number of times Descriptor::remove
   * has been called, this is incremented at the start of Descriptor::remove
   * and decremented upon return.
   */
  uint64_t recursive_depth_ {0};

  /** 
   * help_id_ is a variable used to track which thread to check for an
   * announcement.
   *
   * This is used exclusively by the annoucement table function tryHelpAnother*
   *    *(unless function has been renamed)
   */
  uint64_t help_id_ {0};

  /** 
   * delay_count_ is a variable used to delay how often a thread checks for an
   * annoucnement
   * 
   *  This is used exclusively by the annoucement table function tryHelpAnother*
   *    *(unless function has been renamed)
   */
  uint64_t delay_count_ {0};

  /**
   * Tervel provides a link to the shared Tervel object. This object contians
   * number of threads, hazard_pointer, and other shared structures.
   */
  const Tervel *tervel_ {nullptr};

 public:
  /**
   * This is a link to the threads pool of reference counted descriptor objects.
   */
  const util::memory::rc::DescriptorPool rc_descriptor_pool_;

  /**
   * THis is a link to the threads pool of hp protected elements
   */
  const util::memory::hp::ElementList hp_element_list_;

 private:
  friend RecursiveAction;
  DISALLOW_COPY_AND_ASSIGN(ThreadContext);
};


__thread ThreadContext * tl_thread_info;

namespace util {
/**
 * Helper class for RAII management of recursive helping of threads. Lifetime
 * of this object handles the increment and decrement of the `recursive_depth`
 * of the given ThreadInfo object and sets the `recursive_return` if needed.
 */
class RecursiveAction {
 public:
  RecursiveAction() {
    if (tervel::tl_thread_info->recursive_depth_ >
        tervel::tl_thread_info->get_num_threads() + 1) {
      tervel::tl_thread_info->recursive_return_ = true;
    }
    tervel::tl_thread_info->recursive_depth_ += 1;
  }

  ~RecursiveAction() { tervel::tl_thread_info->recursive_depth_ -= 1; }

 private:
  DISALLOW_COPY_AND_ASSIGN(RecursiveAction);
};

}  // namespace util
}  // namespace tervel

#endif  //  TERVEL_UTIL_INFO_H_
