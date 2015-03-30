#include "tervel/util/thread_context.h"
#include "tervel/util/info.h"
#include "tervel/util/tervel.h"

#include <stdint.h>

namespace tervel {

ThreadContext::ThreadContext(Tervel* tervel)
    : thread_id_ {tervel->get_thread_id()}
    , tervel_ {tervel} {
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
  return rc_descriptor_pool_;
}

util::memory::hp::ElementList* ThreadContext::get_hp_element_list() {
  return &(tervel_->hazard_pointer_->hp_element_list_);
}

uint64_t ThreadContext::get_num_threads() {
  return tervel_->num_threads_;
}

}  // namespace tervel
