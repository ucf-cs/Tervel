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
constexpr size_t CACHE_LINE_SIZE = 64*2;  // bytes
#else
constexpr size_t CACHE_LINE_SIZE = 64;  // bytes
#endif

}  // namespace util
}  // namespace tervel

#endif  // TERVEL_UTIL_SYSTEM_H_
