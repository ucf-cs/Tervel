#ifndef TERVEL_UTIL_THREAD_CONTEXT_H
#define TERVEL_UTIL_THREAD_CONTEXT_H

#include "tervel/util/util.h"


namespace tervel {
class Tervel;

namespace util {

class RecursiveAction;
class ProgressAssurance;

namespace memory {
namespace hp {

class ElementList;
class HazardPointer;

}  // namespace hp

namespace rc {

class DescriptorPool;

}  // namespace rc

}  // namespace memory
}  // namespace util

/**
 * Thread local information. Each thread should have an instance of this.
 */
class ThreadContext {
 public:
  explicit ThreadContext(Tervel* tervel);

  ~ThreadContext() {
    // TODO(steven) delete descriptor pools, return thread id
  }

  /**
   * @returns a reference to the HazardPointer singleton
   */
  util::memory::hp::HazardPointer* get_hazard_pointer();

  /**
   * @returns a reference to the ProgressAssurance singleton
   */
  util::ProgressAssurance* get_progress_assurance();

  /**
   * @returns a reference to the hp_element_list_
   */
  util::memory::hp::ElementList* get_hp_element_list();

  /**
   * @returns a reference to the rc_descriptor_pool
   */
  util::memory::rc::DescriptorPool* get_rc_descriptor_pool();

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
   * increments the recursive depth
   */
  void inc_recursive_depth() {
    recursive_depth_++;
  }

  /**
   * decrements the recrusive depth
   */
  void dec_recursive_depth() {
    recursive_depth_--;
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
  uint64_t get_num_threads();
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
  const util::memory::rc::DescriptorPool* rc_descriptor_pool_;

  /**
   * THis is a link to the threads pool of hp protected elements
   */
  const util::memory::hp::ElementList* hp_element_list_;

 private:
  DISALLOW_COPY_AND_ASSIGN(ThreadContext);
};

}  // namespace tervel
#endif  // TERVEL_UTIL_THREAD_CONTEXT_H
