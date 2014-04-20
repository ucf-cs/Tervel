#include "thread/progress_assurance.h"


namespace ucf {
namespace thread {

void ProgressAssurance::try_to_help(ThreadInfo *local_info) {
  if (local_info->delay_count++ > HELP_DELAY) {
    local_info->delay_count = 0;

    OpRecord *op = op_table_[local_info->help_id].load();
    if (op != nullptr) {
      // std::atomic<void *> *temp = reinterpret_cast<std::atomic<void *> *>(
      //     &(op_table_[local_info->help_id]));

      // TODO(carlos) implement OpRecord for this to work
      // int pos =  hp::ID::id_oprec;
      // bool res = hp::PoolElem::watch(temp, op, pos);
      // if (res) {
      //   if (op->advanceWatch(temp, op, pos)) {
      //     op->helpComplete();
      //     hp::PoolElem::unwatch(op, pos);
      //   }
      // }
    }

    local_info->help_id = (local_info->help_id + 1) % shared_info_.num_threads;
  }
}

void ProgressAssurance::ask_for_help(OpRecord *op, ThreadInfo *local_info) {
  op_table_[local_info->thread_id].store(op);
  // op->helpComplete();  // TODO(carlos) implement OpRecord for this to work
  op_table_[local_info->thread_id].store(nullptr);
}

}  // namespace thread
}  // namespace ucf
