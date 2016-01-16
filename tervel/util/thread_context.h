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
#ifndef TERVEL_UTIL_THREAD_CONTEXT_H
#define TERVEL_UTIL_THREAD_CONTEXT_H

#include <stddef.h>
#include <stdint.h>

#include <tervel/util/util.h>


namespace tervel {
class Tervel;

namespace util {

class RecursiveAction;
class ProgressAssurance;
class EventTracker;

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

  ~ThreadContext();

  /**
   * @returns a reference to the HazardPointer singleton
   */
  util::memory::hp::HazardPointer * get_hazard_pointer();

  /**
   * @returns a reference to the ProgressAssurance singleton
   */
  util::ProgressAssurance * get_progress_assurance();

  /**
   * @returns a reference to the hp_element_list_
   */
  util::memory::hp::ElementList * get_hp_element_list();
  /**
   * @returns a reference to the rc_descriptor_pool
   */
  util::memory::rc::DescriptorPool * get_rc_descriptor_pool();

  util::EventTracker * get_event_tracker();


  /**
   * A unique ID among all active threads.
   * @return the threads id.
   */
  uint64_t get_thread_id();
  /**
   * @return number of threads
   */
  uint64_t get_num_threads();

 private:

  /**
   * Tervel provides a link to the shared Tervel object. This object contains
   * number of threads, hazard_pointer, and other shared structures.
   */
  Tervel * const tervel_;
  const uint64_t thread_id_;
  util::memory::hp::ElementList * const hp_element_list_;
  util::memory::rc::DescriptorPool * const rc_descriptor_pool_;
  util::EventTracker * const eventTracker_;

 private:
  DISALLOW_COPY_AND_ASSIGN(ThreadContext);
};

}  // namespace tervel
#endif  // TERVEL_UTIL_THREAD_CONTEXT_H
