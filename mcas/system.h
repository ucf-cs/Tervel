#ifndef MCAS_SYSTEM_H_
#define MCAS_SYSTEM_H_
/**
 * System-dependent constants.
 */

#include <stddef.h>

namespace ucf {
namespace thread {

constexpr size_t CACHE_LINE_SIZE = 64;  // bytes

}  // namespace thread
}  // namespace ucf

#endif  // MCAS_SYSTEM_H_
