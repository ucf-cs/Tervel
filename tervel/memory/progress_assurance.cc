#include "tervel/memory/progress_assurance.h"

namespace tervel {
namespace memory {

void ProgressAssurance::try_to_help()) {
  if (tl_thread_info->delay_count++ > HELP_DELAY) {
    tl_thread_info->delay_count = 0;

    OpRecord *op = op_table_[tl_thread_info->help_id].load();
    if (op != nullptr) {
      std::atomic<void *> *address = reinterpret_cast<std::atomic<void *> *>(
          &(op_table_[local_info->help_id]));

      int pos =  HazardPointer::SlotID::OPREC;
      bool res = HazardPointer::watch(pos, op, address, op);
      if (res) {
        op->help_complete();
      }
    }
    local_info->help_id = (tl_thread_info->help_id + 1) % tl_thread_info->shared_info_->num_threads;
  }
}

void ProgressAssurance::ask_for_help(OpRecord *op) {
  op_table_[tl_thread_info->thread_id].store(op);
  op->help_complete();
  op_table_[tl_thread_info->thread_id].store(nullptr);
}

}  // namespace memory
}  // namespace tervel
