#include "thread/progress_assurance.h"


namespace ucf {
namespace thread {

void ProgressAssurance::try_to_help() {
  if (tl_thread_info->delay_count++ > HELP_DELAY) {
    tl_thread_info->delay_count = 0;

    OpRecord *op = op_table_[tl_thread_info->help_id].load();
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

    local_info->help_id = (tl_thread_info->help_id + 1) % tl_thread_info->shared_info_->num_threads;
  }
}

void ProgressAssurance::ask_for_help(OpRecord *op) {
  op_table_[tl_thread_info->thread_id].store(op);
  // op->helpComplete();  // TODO(carlos) implement OpRecord for this to work
  op_table_[tl_thread_info->thread_id].store(nullptr);
}

}  // namespace thread
}  // namespace ucf
