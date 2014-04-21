#ifndef UCF_THREAD_PROGRESS_ASSURANCE_H_
#define UCF_THREAD_PROGRESS_ASSURANCE_H_

#include <atomic>
#include <memory>

#include "thread/info.h"


namespace ucf {
namespace thread {

constexpr size_t HELP_DELAY = 1;

// TODO(carlos) originally, this extends hp::PoolElem. Why?
// It should be again because these are HP protected objects
class OpRecord {
 public:
  static constexpr size_t MAX_FAILURES = 1;

  virtual ~OpRecord() {}

  virtual void help_complete() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(OpRecord);
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

  DISALLOW_COPY_AND_ASSIGN(ProgressAssurance);
};

}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_PROGRESS_ASSURANCE_H_
