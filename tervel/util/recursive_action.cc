#include <tervel/util/recursive_action.h>


namespace tervel{
namespace util {

  size_t RecursiveAction::recursive_depth(size_t i) {
    static __thread size_t recursive_depth_count = 0;
    return recursive_depth_count += i;
  }

  bool RecursiveAction::recursive_return(bool change, bool value) {
    static __thread bool recursive_return_ = false;
    if (change) {
      recursive_return_ = value;
    }
    return recursive_return_;
  }

}  // namespace util
}  // namespace tervel
