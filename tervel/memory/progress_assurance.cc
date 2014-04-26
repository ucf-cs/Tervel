#include "tervel/memory/progress_assurance.h"

namespace tervel {
namespace memory {

// REVIEW(carlos): extra paren
void ProgressAssurance::p_try_to_help()) {
  // Internally, delay_count is incremented and set to 0 when ever HELP_DELAY
  // is reached
  int delay_count = tl_thread_info->delay_count(HELP_DELAY);
  if (delay_count == 0) {
    // Internally, help_id is incremented and wrapped to number of threads.
    int help_id = tl_thread_info->help_id(num_threads_);
    OpRecord *op = op_table_[help_id].load();
    if (op != nullptr) {
      // REVIEW(carlos): Don't bother trying to line up a line continuation;
      //   just indent the next line by 2 indentation levels (4 spaces). When
      //   splitting a line, prefer to leave an opening brace or operator at the
      //   end of the previous line:
      //     ... address = reinterpret_cast<...>(
      //         &(...));
      //   This gives a visual cue to the reader that the line is being
      //   continued.
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

// REVIEW(carlos): Default parameters should go in method declaration (.h
//   file) since they serve as documentation for what the default case is. Don't
//   also put them in the .cc file because then changes to one file must always
//   be manually mirrored.
// REVIEW(carlos): Don't bother trying to line up broken lines, just move to
//   next line and indent 2 indentation levels (4 spaces). Ex:
//     void this_is_a_long_function_name(int long_argument_name_too=
//         default_argument);
//   I prefer no spaces around the `=' if it's a default parameter, but either
//   way is fine.
void ProgressAssurance::p_make_annoucment(OpRecord *op, int tid =
                                                    tl_thread_info.thread_id) {
  op_table_[tid].store(op);
  op->help_complete();
  op_table_[tid].store(nullptr);
}
// REVIEW(carlos): Excessive vertical whitespace at before closing of namespace.
//   Please try to limit vertical whitespace when possible so that more code is
//   visible on screen (rule of thumb: usually just 1 blank line for
//   readability, 2 blank lines to mark the beginning of a logical section)
//   Here, one blank line before the closing brackets suffice.



}  // namespace memory
}  // namespace tervel
