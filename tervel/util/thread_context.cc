#include "tervel/util/thread_context.h"
#include "tervel/util/info.h"
#include "tervel/util/tervel.h"

#include <stdint.h>

namespace tervel {

ThreadContext::ThreadContext(Tervel* tervel)
    : tervel_ {tervel} {
  this->get_thread_id();  // init thread id
  tl_thread_info = this;
  rc_descriptor_pool_ = tervel->rc_pool_manager_.allocate_pool();
  hp_element_list_ = tervel->hp_list_manager_.allocate_list();
}

util::memory::hp::HazardPointer* ThreadContext::get_hazard_pointer() {
  return &(tervel_->hazard_pointer_);
}

util::ProgressAssurance* ThreadContext::get_progress_assurance() {
  return &(tervel_->progress_assurance_);
}


util::memory::rc::DescriptorPool* ThreadContext::get_rc_descriptor_pool() {
  static __thread util::memory::rc::DescriptorPool* rc_descriptor_pool_ =
    tervel->rc_pool_manager_.allocate_pool();
  return rc_descriptor_pool_;
}

util::memory::hp::ElementList* ThreadContext::get_hp_element_list() {
  static __thread util::memory::hp::ElementList* hp_element_list_ =
    tervel->hp_list_manager_.allocate_list();
  return hp_element_list_;
}

uint64_t get_thread_id() {
  static __thread uint64_t thread_id_ = tervel->get_thread_id();
  return thread_id_;
}

uint64_t ThreadContext::get_num_threads() {
  return tervel_->num_threads_;
}

static size_t recrusive_depth(size_t i) {
  static __thread size_t recursive_depth_count = 0;
  return recursive_depth_count += i;
}

static bool recursive_return(bool change) {
  static __thread bool recrusive_return_;
  if (change) {
    recrusive_return_ = value;
  }
  return recursive_return_;
}

}  // namespace tervel
