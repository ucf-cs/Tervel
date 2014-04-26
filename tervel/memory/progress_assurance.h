#ifndef TERVEL_MEMORY_PROGRESS_ASSURANCE_H_
#define TERVEL_MEMORY_PROGRESS_ASSURANCE_H_

#include <atomic>
#include <memory>

#include "tervel/memory/info.h"
#include "tervel/util.h"


namespace tervel {
namespace memory {
// REVIEW(carlos): Suggest 1 blank line between namespace blocks declaration and
//   beginning of code
/**
 * This class is used to create Operation Records.
 * Operation records are designed to allow an arbitary thread to complete
 * some other thread's operation in the event that thread is unable to do so.
 * These objects are HP protected.
 */
// REVIEW(carlos): There is no `namespace HP' (capital letters)
// REVIEW(carlos): why is OpRecord a subclass of HPElement? shouldn't we keep
//   the two classes seperate? Otherwise, you're explicitly saying that the
//   progress assurance relies on hazard pointere'd memory (which is fine, but
//   if that's the case, progress assurance should be in a new namespace).
// REVIEW(carlos): There's an op_record.h file, too. One of these doesn't
//   belong. Which is it?
class OpRecord : public HP::HPElement {
 public:
  virtual ~OpRecord() {}

  /**
   * Implementations of this function that upon its return the operation 
   * described in the OpRecord has been completed.
   * As such it must be thread-safe and the extending class must contain all the
   * information necessary to complete the operation.
   */
  virtual void help_complete() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(OpRecord);
};

/**
 * This class represents the progress assurance scheme employed by our library.
 * For this scheme to be effective, each operation which may indefinetly prevent
 * the progress of some other operation must call the static function offer_help
 * This ensures that if a thread is continually failing its operation, then
 * after a finite number of tries all thread will be helping.
 * The number of failures is MAX_FAILURES + (number of threads^2)* help_delay
 */
class ProgressAssurance {
 public:
  // Const used to delay an announcement
  static constexpr size_t MAX_FAILURES = 1;
  // REVIEW(carlos): When adding comments, I prefer to put a blank line before
  //   the beginning of a comment block. Otherwise, It looks, at a glance, that
  //   the comment block could apply to the above code when it usually only
  //   belongs to the below code.
  // REVIEW(carlos): For documentation comments, use the javadoc style /** */
  //   comments.
  // Const used to reduce the number of times a thread checks the table
  // Reduces memory loads at the cost of a higher upper bound
  static constexpr size_t HELP_DELAY = 1;

  // REVIEW(carlos): Don't bother trying to line up broken lines, just move to
  //   next line and indent 2 indentation levels (4 spaces). Ex:
  //     void this_is_a_long_function_name(int long_argument_name_too=
  //         default_argument);
  //   I prefer no spaces around the `=' if it's a default parameter, but either
  //   way is fine.
  explicit ProgressAssurance(int num_threads =
                                      tl_thread_info.shared_info_->num_threads)
      : num_threads_ {num_threads}
      , op_table_(new std::atomic<OpRecord *>[num_threads_] ) {}

  /**
   * This function checks at most one position in the op_table_ for an OPRecod
   * If one is found it will call its help_complete function.
   */
  // REVIEW(carlos): See above comment on indenting broken lines.
  // REVIEW(carlos): I tend to prefer top-level functions over static functions.
  //   There isn't much difference between the two, and static functions force
  //   you to write the class name everytime you want to call it. If the
  //   function is very strongly tied to the class, then static functions may be
  //   appropriate, but I'd suggest treating them as the exception, rather than
  //   the rule.
  static void check_for_announcement(ProgressAssurance *progress_assuarance =
                      tl_thread_info.progress_assuarance) {
    progress_assuarance->p_check_for_announcement();
  }

  /**
   * This function places the
   * @param op an OpRecord to complete
   * @return on return the OpRecord must be completed.
   */
  // REVIEW(carlos): See above comments on indenting broken lines. Typically,
  //   the "start the line with the comma" only applies to long argument lists
  //   (like constructor initializers) to even out visual weight. On functions,
  //   I'd put the comma on the arguemnt.
  static void make_announcement(OpRecord *op, int tid = tl_thread_info.thread_id
                                , ProgressAssurance *progress_assuarance =
                                          tl_thread_info.progress_assuarance) {
    progress_assuarance->p_make_announcement(op, tid);
  }

 private:
  /**
   * This function checks at most one position in the op_table_ for an OPRecod
   * If one is found it will call its help_complete function.
   */
  void p_check_for_announcement();

  /**
   * This function places the
   * @param op an OpRecord to complete
   * @return on return the OpRecord must be completed.
   */

  // REVIEW(carlos): There shouldn't be a space between the documentation
  //   comment and the function declarati
  void p_make_announcement(OpRecord *op, int tid = tl_thread_info.thread_id);

  /**
   * Table for storing operation records, each thread has its own position
   * that corresponds to its thread if.
   */
  std::unique_ptr<std::atomic<OpRecord *>[]> op_table_;

  // REVIEW(carlos): For documentation comments, please use javadoc style /** */
  //   comments
  // The number of threads that are using this operation table
  int num_threads_;

  DISALLOW_COPY_AND_ASSIGN(ProgressAssurance);
};

}  // namespace memory
}  // namespace tervel

#endif  // TERVEL_MEMORY_PROGRESS_ASSURANCE_H_
