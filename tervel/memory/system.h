#ifndef TERVEL_MEMORY_SYSTEM_H_
#define TERVEL_MEMORY_SYSTEM_H_
/**
 * System-dependent constants.
 */

#include <stddef.h>

#define DEBUG_POOL true

namespace tervel {
namespace memory {

constexpr size_t CACHE_LINE_SIZE = 64;  // bytes

}  // namespace memory
}  // namespace tervel

#endif  // TERVEL_MEMORY_SYSTEM_H_
