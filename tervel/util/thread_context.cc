#include "tervel/util/thread_context"
#include "tervel/util/info.h"

namespace tervel {

ThreadContext(Tervel* tervel)
      : thread_id_ {tervel->get_thread_id()}
      , tervel_ {tervel}
      , rc_descriptor_pool_(tervel_->rc_pool_manager_)
      , hp_element_list_(tervel_->hp_list_manager_) {
        tl_thread_info = this;
      }

util::memory::hp::HazardPointer* get_hazard_pointer() {
  return &(tervel_->hazard_pointer_);
}

util::ProgressAssurance* get_progress_assurance() {
  return &(tervel_->progress_assurance_);
}


util::memory::hp::ElementList* get_hp_element_list() {
  return rc_descriptor_pool_;
}

util::memory::hp::ElementList* get_rc_descriptor_pool() {
  return hp_element_list_
}

uint64_t get_num_threads() {
  return tervel_->num_threads_;
}

}  // namespace tervel
