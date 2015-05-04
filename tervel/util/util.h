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
#ifndef __TERVEL_UTIL_UTIL_H_
#define __TERVEL_UTIL_UTIL_H_

#include <chrono>
#include <thread>
#include <cmath>

namespace tervel {
namespace util {

//Memory Protection Macros:

// #define TERVEL_MEM_NO_FREE
// #define TERVEL_MEM_NO_WATCH
// caused both RC and HP versions to be defined
#ifdef TERVEL_MEM_NO_FREE
  #define TERVEL_MEM_HP_NO_FREE
  #define TERVEL_MEM_RC_NO_FREE
#endif
#ifdef TERVEL_MEM_NO_WATCH
  #define TERVEL_MEM_HP_NO_WATCH
  #define TERVEL_MEM_RC_NO_WATCH
#endif

// #define TERVEL_MEM_HP_NO_FREE
  // causes safeDelete to return without freeing the object
  // its destructor is NOT called.

// #define TERVEL_MEM_HP_NO_WATCH
/* causes watch functions to return success
 * causes is_watch to return false
 * causes unwatch to return immediately
*/
#ifdef TERVEL_MEM_HP_NO_WATCH
  #ifndef TERVEL_MEM_HP_NO_FREE
    #define TERVEL_MEM_HP_NO_FREE
  #endif
#endif

// #define TERVEL_MEM_RC_NO_FREE
// -causes new objects to be allocated from the allocator

// #define TERVEL_MEM_RC_NO_WATCH
/* causes watch functions to return success
 * causes is_watch to return false
 * causes unwatch to return immediately
*/

// #define TERVEL_MEM_RC_MAX_NODES
 // if the number of nodes in the pool > then this value then the excess is sent to the shared pool
 // excess is determined by TERVEL_MEM_RC_MIN_NODES
#ifndef TERVEL_MEM_RC_MAX_NODES
 #define TERVEL_MEM_RC_MAX_NODES 25
#endif
#ifndef TERVEL_MEM_RC_MIN_NODES
 #define TERVEL_MEM_RC_MIN_NODES 5
#endif



// TERVEL Progress Assurance MACROS:

// #define TERVEL_PROG_ASSUR_DELAY
// sets the delay between calling the check for announcement function
#ifndef TERVEL_PROG_ASSUR_DELAY
 #define TERVEL_PROG_ASSUR_DELAY 100
#endif

// #define TERVEL_PROG_ASSUR_NO_CHECK
// sets the delay to -1, which makes it never reach 0 and never triggers the call
// if this is set then set TERVEL_PROG_NO_ANNOUNCE
#ifdef TERVEL_PROG_ASSUR_NO_CHECK
 #ifdef TERVEL_PROG_ASSUR_DELAY
  #undef TERVEL_PROG_ASSUR_DELAY
 #endif
 #define TERVEL_PROG_ASSUR_DELAY -1
 #define TERVEL_PROG_NO_ANNOUNCE
#endif


// #define TERVEL_PROG_ASSUR_ALWAYS_CHECK
 // sets the delay to 0, which makes it always trigger
#ifdef TERVEL_PROG_ASSUR_ALWAYS_CHECK
 #ifdef TERVEL_PROG_ASSUR_DELAY
  #undef TERVEL_PROG_ASSUR_DELAY
 #endif
 #define TERVEL_PROG_ASSUR_DELAY 0
#endif

#ifdef TERVEL_PROG_ASSUR_NO_CHECK
  #error TERVEL_PROG_ASSUR_NO_CHECK and TERVEL_PROG_ASSUR_ALWAYS_CHECK can not both be set.
#endif

// #define TERVEL_PROG_ASSUR_LIMIT
  // sets the delay before making an announcement
#ifndef TERVEL_PROG_ASSUR_LIMIT
  #define TERVEL_PROG_ASSUR_LIMIT 1000
#endif

// #define TERVEL_PROG_ASSUR_NO_ANNOUNCE
  // disables the making of an announcement
  // sets TERVEL_PROG_ASSUR_LIMIT = -1

#ifdef TERVEL_PROG_ASSUR_NO_ANNOUNCE
  #ifdef TERVEL_PROG_ASSUR_LIMIT
    #undef TERVEL_PROG_ASSUR_LIMIT
  #endif
  #define TERVEL_PROG_ASSUR_LIMIT -1
#endif

// #define TERVEL_PROG_ASSUR_ALWAYS_ANNOUNCE
  // always use progress assurance
  // sets TERVEL_PROG_ASSUR_LIMIT = 0
#ifdef TERVEL_PROG_ASSUR_ALWAYS_ANNOUNCE
  #ifdef TERVEL_PROG_ASSUR_LIMIT
    #undef TERVEL_PROG_ASSUR_LIMIT
  #endif
  #define TERVEL_PROG_ASSUR_LIMIT 0
#endif


/**
 * @brief Returns whether or not the passed value is has one of the reserved bits set
 * to 1.
 * @details Returns whether or not the passed value is has one of the reserved bits set
 * to 1.
 *
 * @param value the value to check
 * @return whether or not value is valid
 */
inline bool isValid(void * value) {
  uintptr_t temp = reinterpret_cast<uintptr_t>(value);
  return !(temp & 3);
}


#ifndef TERVEL_DEF_BACKOFF_TIME_NS
 #define TERVEL_DEF_BACKOFF_TIME_NS 10000
#endif
/**
 * @brief Sets the amount of time in nano-seconds for a thread to backoff before
 * re-retrying.
 * @details Sets the amount of time in nano-seconds for a thread to backoff before
 * re-retrying.
 *
 * @param duration duration
 */
inline void backoff(int duration = TERVEL_DEF_BACKOFF_TIME_NS) {
  std::this_thread::sleep_for(std::chrono::nanoseconds(duration));
}

/**
 * @brief Returns the next power of two
 * @details Returns the next power of two, if a power of two is passed, then
 * that value is returned
 *
 * TODO(steven): replace with a bit function
 *
 * @param value the value to find the next power of two
 * @return  the next power of two
 */
inline int round_to_next_power_of_two(uint64_t value) {
  double val = std::log2(value);
  int int_val = static_cast<int>(val);
  if (int_val < val) {
    int_val++;
  }
  return int_val;
};

}  // namespace util
}  // namespace tervel


// A macro to disallow the copy constructor and operator= functions.  This
// should be used in the `private` declarations for a class. Use unless you have
// a good reason for a class to be copy-able.
// see:
//   http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Copy_Constructors
//   http://stackoverflow.com/questions/20026445
#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
  TypeName(const TypeName&) = delete;       \
  void operator=(const TypeName&) = delete

#endif  // __TERVEL_UTIL_UTIL_H_
