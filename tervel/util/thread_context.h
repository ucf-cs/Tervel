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
