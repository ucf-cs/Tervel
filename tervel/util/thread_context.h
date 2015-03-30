#ifndef TERVEL_UTIL_THREAD_CONTEXT_H
#define TERVEL_UTIL_THREAD_CONTEXT_H

#include <stddef.h>
#include <stdint.h>

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
  util::memory::hp::HazardPointer * const get_hazard_pointer();

  /**
   * @returns a reference to the ProgressAssurance singleton
   */
  util::ProgressAssurance * const get_progress_assurance();

  /**
   * @returns a reference to the hp_element_list_
   */
  util::memory::hp::ElementList * const get_hp_element_list();
  /**
   * @returns a reference to the rc_descriptor_pool
   */
  util::memory::rc::DescriptorPool * const get_rc_descriptor_pool();




/**
 * The following functions handle the counting of the recursive depth.
 * This structure is designed to optimize the thread local memory access
 */
private:
  static size_t recursive_depth(size_t i);

public:
  /**
   * Recurive_return functions: Used to indicate a thread must return to its own
   * operation and re-evaluate its state. This is set to true in the event
   * 1) The thread reasons that the dependency between the current op it is
   * Trying to help has changed and a result it must re-examine its op
   * 2) Max Fail count has been reached and it needs to make an announcement
   * For its operation
   *
   * recursive_depth: used to track the number of times Descriptor::remove
   * has been called, this is incremented at the start of Descriptor::remove
   * and decremented upon return.
   */

  /**
   * @return the current recursive depth
   */
  static size_t get_recursive_depth();

  /**
   * increments the recursive depth
   */
  static void inc_recursive_depth();

  /**
   * decrements the recursive depth
   */
  static void dec_recursive_depth();

  /**
  * @return whether or not the thread is performing a recursive return.
  */
  static bool recursive_return(bool change = false, bool value = false);

  /**
   * Sets recrusive_return_ to true
   */
  static void set_recursive_return() {
    recursive_return(true, true);
  }

  /**
   * Sets recrusive_return_ to false
   */
  static void clear_recursive_return() {
    recursive_return(true, false);
  }

// End Recursive Return functions

  /**
   * A unique ID among all active threads.
   * @return the threads id.
   */
  const uint64_t get_thread_id();
  /**
   * @return number of threads
   */
  const uint64_t get_num_threads();

 private:

  /**
   * Tervel provides a link to the shared Tervel object. This object contains
   * number of threads, hazard_pointer, and other shared structures.
   */
  Tervel * const tervel_;
  const uint64_t thread_id_;
  util::memory::hp::ElementList * const hp_element_list_;
  util::memory::rc::DescriptorPool * const rc_descriptor_pool_;

 private:
  DISALLOW_COPY_AND_ASSIGN(ThreadContext);
};

}  // namespace tervel
#endif  // TERVEL_UTIL_THREAD_CONTEXT_H
