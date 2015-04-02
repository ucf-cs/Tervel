#ifndef DESCRIPTOR_READ_FIRST_OP_H_
#define DESCRIPTOR_READ_FIRST_OP_H_

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/rc/descriptor_util.h>

namespace tervel {
namespace util {

class Descriptor;

namespace memory {
namespace rc {

/**
 * Class used for placement in the Op Table to complete an operation that failed
 *    to complete in a bounded number of steps
 */
class ReadFirstOp : public util::OpRecord {
 public:
  explicit ReadFirstOp(std::atomic<void *> *address) {
    address_ = address;
    value_.store(nullptr);
  }

  ~ReadFirstOp() {}

  /**
   * This function overrides the virtual function in the OpRecord class
   * It is called by the progress assurance scheme.
   */
  void help_complete();

  void *load();

 private:
  std::atomic<void *> *address_;
  std::atomic<void *> value_;
};  // ReadFirstOp class

}  // namespace rc
}  // namespace memory
}  // namespace util
}  // namespace tervel

#endif  // DESCRIPTOR_READ_FIRST_OP_H_
