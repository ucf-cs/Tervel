#ifndef UCF_THREAD_PROGRESS_ASSURANCE_H_
#define UCF_THREAD_PROGRESS_ASSURANCE_H_

#include <atomic>
#include <memory>

#include "thread/info.h"


namespace ucf {
namespace thread {

constexpr size_t HELP_DELAY = 1;

// TODO(carlos) originally, this extends hp::PoolElem. Why?
class OpRecord {
 public:
  static constexpr size_t MAX_FAILURES = 1;

  virtual ~OpRecord() {}

  virtual void help_complete() = 0;
};

class ProgressAssurance {
 public:
  explicit ProgressAssurance(const SharedInfo &shared_info)
      : shared_info_(shared_info)
      , op_table_(new std::atomic<OpRecord *>[shared_info_.num_threads]) {}

  // TODO(carlos) what do these do?
  void try_to_help(ThreadInfo *local_info);
  void ask_for_help(OpRecord *op, ThreadInfo *local_info);

 private:
  const SharedInfo &shared_info_;
  std::unique_ptr<std::atomic<OpRecord *>[]> op_table_;
};

}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_PROGRESS_ASSURANCE_H_
