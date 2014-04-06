#include "descriptor.h"
#include "pool_element.h"

namespace ucf {
namespace thread {
namespace rc {


void PoolElem::cleanup_descriptor() {
#ifdef POOL_DEBUG
  assert(header_.descriptor_in_use_.load());
  header_.descriptor_in_use_.store(false);
#endif
  this->descriptor()->~Descriptor();
}


}  // namespace rc
}  // namespace thread
}  // namespace ucf
