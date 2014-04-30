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
    if (tervel::tl_thread_info->recursive_depth_ >
        tervel::tl_thread_info->get_num_threads() + 1) {
      tervel::tl_thread_info->recursive_return_ = true;
    }
    tervel::tl_thread_info->recursive_depth_ += 1;
  }

  ~RecursiveAction() { tervel::tl_thread_info->recursive_depth_ -= 1; }

 private:
  DISALLOW_COPY_AND_ASSIGN(RecursiveAction);
};

}  // namespace util
}  // namespace tervel
#end  // TERVEL_UTIL_RECURSIVE_ACTION_H