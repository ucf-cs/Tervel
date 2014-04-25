#include "tervel/memory/progress_assurance.h"

namespace tervel {
namespace memory {

void ProgressAssurance::p_try_to_help()) {
  // Internally, delay_count is incremented and set to 0 when ever HELP_DELAY
  // is reached
  int delay_count = tl_thread_info->delay_count(HELP_DELAY);
  if (delay_count == 0) {
    // Internally, help_id is incremented and wrapped to number of threads.
    int help_id = tl_thread_info->help_id(num_threads_);
    OpRecord *op = op_table_[help_id].load();
    if (op != nullptr) {
      std::atomic<void *> *address = reinterpret_cast<std::atomic<void *> *>
                                                        (&(op_table_[help_id]));

      int pos =  HazardPointer::SlotID::OPREC;
      bool res = HazardPointer::watch(pos, op, address, op);
      if (res) {
        op->help_complete();
      }
    }
  }
}

void ProgressAssurance::p_make_annoucment(OpRecord *op, int tid =
                                                    tl_thread_info.thread_id) {
  op_table_[tid].store(op);
  op->help_complete();
  op_table_[tid].store(nullptr);
}



}  // namespace memory
}  // namespace tervel
