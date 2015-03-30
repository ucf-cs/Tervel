#ifndef TERVEL_UTIL_RECURSIVE_ACTION_H
#define TERVEL_UTIL_RECURSIVE_ACTION_H

#include "tervel/util/info.h"
#include "tervel/util/util.h"

namespace tervel{
namespace util {
/**
 * Helper class for RAII management of recursive helping of threads. Lifetime
 * of this object handles the increment and decrement of the `recursive_depth`
 * of the given ThreadInfo object and sets the `recursive_return` if needed.
 */
class RecursiveAction {
 public:
  RecursiveAction() {
    if (tervel::ThreadContext::get_recursive_depth() >
        tervel::tl_thread_info->get_num_threads() + 1) {
      tervel::ThreadContext::set_recursive_return();
    }
    tervel::ThreadContext::inc_recursive_depth();
  }

  ~RecursiveAction() {
    tervel::ThreadContext::dec_recursive_depth();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(RecursiveAction);
};  // class RecursiveAction

}  // namespace util
}  // namespace tervel
#endif  // TERVEL_UTIL_RECURSIVE_ACTION_H