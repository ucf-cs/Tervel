/**
 * This file defines some lightweight structures for manipulating the shared and
 * thread-local information which threads should have access to when running.
 */
#ifndef TERVEL_UTIL_INFO_H_
#define TERVEL_UTIL_INFO_H_

#include <atomic>
#include <stdint.h>

#include "tervel/util/thread_context.h"
namespace tervel {
class ThreadContext;

extern thread_local ThreadContext * tl_thread_info {nullptr};
}  // namespace tervel



#endif  //  TERVEL_UTIL_INFO_H_
