#ifndef TERVEL_UTIL_SYSTEM_H_
#define TERVEL_UTIL_SYSTEM_H_
/**
 * System-dependent constants.
 */

#include <stddef.h>

//#define DEBUG_POOL true

namespace tervel {
namespace util {

#if DEBUG_POOL
#define CACHE_LINE_SIZE 64*2
#else
#define  CACHE_LINE_SIZE 64
#endif

}  // namespace util
}  // namespace tervel

#endif  // TERVEL_UTIL_SYSTEM_H_
