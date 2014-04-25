#ifndef TERVEL_MEMORY_PROGRESS_ASSURANCE_H_
#define TERVEL_MEMORY_PROGRESS_ASSURANCE_H_

#include <atomic>
#include <memory>

#include "tervel/memory/info.h"


namespace tervel {
namespace memory {

constexpr size_t HELP_DELAY = 1;

// REVIEW(carlos) If you feel this is the correct way forward, just implement
//   it. If you're not sure, the comment is fine, but do something to mark it as
//   a continuation of the TODO.
// TODO(carlos) originally, this extends hp::PoolElem. Why?
// It should be again because these are HP protected objects
class OpRecord {
 public:
  static constexpr size_t MAX_FAILURES = 1;

  virtual ~OpRecord() {}

  virtual void help_complete() = 0;
};

class ProgressAssurance {
 public:
  explicit ProgressAssurance()
      : op_table_(
        new std::atomic<OpRecord *>[tl_thread_info.shared_info_->num_threads]
        ) {}

  // TODO(carlos) what do these do?
  void try_to_help();
  void ask_for_help(OpRecord *op);

 private:
  std::unique_ptr<std::atomic<OpRecord *>[]> op_table_;
};

}  // namespace memory
}  // namespace tervel

#endif  // TERVEL_MEMORY_PROGRESS_ASSURANCE_H_
