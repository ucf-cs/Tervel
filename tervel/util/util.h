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
// casused both RC and HP versions to be defined
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

//TODO(Steven): make a new class ProgAssur to replace fail count variable coding pattern
// ProgAssur(int limit=TERVEL_PROG_ASSUR_LIMIT); bool ProgAssur::delayed(int rate=1)

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
 * Returns whether or not the passed value is has one of the reserved bits set
 * to 1.
 *
 */
inline bool isValid(void * value) {
  uintptr_t temp = reinterpret_cast<uintptr_t>(value);
  return !(temp & 3);
}

/**
 * TODO comment
 *  TODO(steven): add the default value compile time constant documentation
 */
#ifndef TERVEL_DEF_BACKOFF_TIME_NS
 #define TERVEL_DEF_BACKOFF_TIME_NS 10000
#endif
inline void backoff(int duration = TERVEL_DEF_BACKOFF_TIME_NS) {
  std::this_thread::sleep_for(std::chrono::nanoseconds(duration));
}

// TODO(steven): Replace with a bit hack?
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
