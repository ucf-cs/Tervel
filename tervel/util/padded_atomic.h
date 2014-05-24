#ifndef TERVEL_UTIL_PADDEDATOMIC_H
#define TERVEL_UTIL_PADDEDATOMIC_H

#include "tervel/util/system.h"
#include "tervel/wf-ring-buffer/node.h"

#include <atomic>

namespace tervel {
namespace util {

template<class T>
class PaddedAtomic {
 public:
  explicit PaddedAtomic(wf_ring_buffer::Node<T> *atomic) {
    this.atomic.store(atomic);
    this.padding = new char[CACHE_LINE_SIZE-sizeof(atomic)];
  }

  std::atomic<wf_ring_buffer::Node<T> *> atomic;

 private:
  char padding[];
};

}  // namespace util
}  // namespace tervel

#endif  // TERVEL_UTIL_PADDEDATOMIC_H
