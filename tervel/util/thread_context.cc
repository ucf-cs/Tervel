/*
The MIT License (MIT)

Copyright (c) 2015 University of Central Florida's Computer Software Engineering
Scalable & Secure Systems (CSE - S3) Lab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <tervel/util/thread_context.h>
#include <tervel/util/info.h>
#include <tervel/util/tervel.h>
#include <tervel/util/memory/hp/hp_list.h>
#include <tervel/util/memory/rc/descriptor_pool.h>
#include <tervel/util/tervel_metrics.h>

#include <stdint.h>

namespace tervel {

ThreadContext::ThreadContext(Tervel* tervel)
    : tervel_ {tervel}
    , thread_id_(tervel_->get_thread_id())
    , hp_element_list_(tervel_->hazard_pointer_.hp_list_manager_.allocate_list())
    , rc_descriptor_pool_(tervel_->rc_pool_manager_.allocate_pool(thread_id_))
    , eventTracker_(new util::EventTracker()){
  tl_thread_info = this;
  tervel->thread_contexts_[thread_id_] = this;

}

ThreadContext::~ThreadContext() {
  if (rc_descriptor_pool_ != nullptr) {
    delete rc_descriptor_pool_;
  }
  if (hp_element_list_ != nullptr) {
    delete hp_element_list_;
  }

  tl_thread_info = nullptr;
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

util::EventTracker* const ThreadContext::get_event_tracker() {
  return eventTracker_;
}


}  // namespace tervel
