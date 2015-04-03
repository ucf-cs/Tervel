#ifndef TERVEL_UTIL_RECURSIVE_ACTION_H
#define TERVEL_UTIL_RECURSIVE_ACTION_H

#include <tervel/util/info.h>
#include <tervel/util/util.h>

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
    if (RecursiveAction::recursive_depth(0) >
        tervel::tl_thread_info->get_num_threads() + 1) {
      RecursiveAction::recursive_return(true, true);
    }
    RecursiveAction::recursive_depth(1);
  }

  ~RecursiveAction() {
    RecursiveAction::recursive_depth(-1);
  }

  /**
  * @return whether or not the thread is performing a recursive return.
  */
  static bool recursive_return(bool change = false, bool value = false);

  static void set_recursive_return() {
    recursive_return(true, true);
  };

  static void clear_recursive_return() {
    recursive_return(true, false);
  };

  /**
   * Adds the passed value and returns the pre-incremented value
   * @param  i value to increment by
   * @return   the value before the increment
   */
  static size_t recursive_depth(size_t i = 0);

 private:
  DISALLOW_COPY_AND_ASSIGN(RecursiveAction);
};  // class RecursiveAction

}  // namespace util
}  // namespace tervel
#endif  // TERVEL_UTIL_RECURSIVE_ACTION_H