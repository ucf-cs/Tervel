#include "tervel/util/thread_context.h"
#include "tervel/util/info.h"
#include "tervel/util/tervel.h"

#include <stdint.h>

namespace tervel {

ThreadContext::ThreadContext(Tervel* tervel)
    : tervel_ {tervel}
    , thread_id_(tervel_->get_thread_id())
    , hp_element_list_(tervel_->hazard_pointer_.hp_list_manager_.allocate_list())
    , rc_descriptor_pool_(tervel_->rc_pool_manager_.allocate_pool(thread_id_)) {
  tl_thread_info = this;
}

util::memory::hp::HazardPointer * const ThreadContext::get_hazard_pointer() {
  return &(tervel_->hazard_pointer_);
}

util::ProgressAssurance * const ThreadContext::get_progress_assurance() {
  return &(tervel_->progress_assurance_);
}

util::memory::rc::DescriptorPool * const ThreadContext::get_rc_descriptor_pool() {
  return rc_descriptor_pool_;
}

util::memory::hp::ElementList * const ThreadContext::get_hp_element_list() {
  return hp_element_list_;
}

const uint64_t ThreadContext::get_thread_id() {
  return thread_id_;
}

const uint64_t ThreadContext::get_num_threads() {
  return tervel_->num_threads_;
}


}  // namespace tervel
