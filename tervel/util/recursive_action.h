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
#ifndef TERVEL_UTIL_RECURSIVE_ACTION_H
#define TERVEL_UTIL_RECURSIVE_ACTION_H

#include <tervel/util/info.h>
#include <tervel/util/util.h>

namespace tervel{
namespace util {
/**
 * Helper class for RAII management of recursive helping of threads. Lifetime
 * of this object handles the increment and decrement of the `recursive_depth`
 * of the given ThreadInfo object and sets the `recursive_return` if needed.
 */
class RecursiveAction {
 public:
  RecursiveAction() {
    if (RecursiveAction::recursive_depth(0) >
        tervel::tl_thread_info->get_num_threads() + 1) {
      #ifdef tervel_track_max_recur_depth_reached
        util::EventTracker::countEvent(util::EventTracker::event_code::max_recur_depth_reached);
      #endif


      RecursiveAction::recursive_return(true, true);
    }
    RecursiveAction::recursive_depth(1);
  }

  ~RecursiveAction() {
    RecursiveAction::recursive_depth(-1);
  }

  /**
  * @return whether or not the thread is performing a recursive return.
  */
  static bool recursive_return(bool change = false, bool value = false);

  static void set_recursive_return() {
    recursive_return(true, true);
  };

  static void clear_recursive_return() {
    recursive_return(true, false);
  };

  /**
   * Adds the passed value and returns the pre-incremented value
   * @param  i value to increment by
   * @return   the value before the increment
   */
  static size_t recursive_depth(size_t i = 0);

 private:
  DISALLOW_COPY_AND_ASSIGN(RecursiveAction);
};  // class RecursiveAction

}  // namespace util
}  // namespace tervel
#endif  // TERVEL_UTIL_RECURSIVE_ACTION_H