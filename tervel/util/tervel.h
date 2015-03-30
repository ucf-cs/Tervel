#ifndef TERVEL_UTIL_TERVEL_H
#define TERVEL_UTIL_TERVEL_H

#include "tervel/util/util.h"
#include "tervel/util/thread_context.h"
#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/hp/hazard_pointer.h"
#include "tervel/util/memory/rc/pool_manager.h"

namespace tervel {

/**
 * Contains shared information that should be accessible by all threads.
 */
class Tervel {
 public:
  explicit Tervel(size_t num_threads)
      : num_threads_  {num_threads}
      , hazard_pointer_(num_threads)
      , rc_pool_manager_(num_threads)
      , progress_assurance_(num_threads) {}

  ~Tervel() {
    // TODO(steven) implement
  }

 private:
  uint64_t get_thread_id() {
    return active_threads_.fetch_add(1);
  }

  // The total number of expected threads in the system.
  uint64_t num_threads_;

  // The number of threads which have been assigned an thread_id
  std::atomic<uint64_t> active_threads_ {0};

  // The shared hazard_pointer object
  util::memory::hp::HazardPointer hazard_pointer_;

  // Shared RC Descriptor Pool Manager
  util::memory::rc::PoolManager rc_pool_manager_;

  // Shared Progress Assurance Object
  util::ProgressAssurance progress_assurance_;

  friend ThreadContext;

  DISALLOW_COPY_AND_ASSIGN(Tervel);
};

}  // namespace tervel
#endif  // TERVEL_UTIL_TERVEL_H
