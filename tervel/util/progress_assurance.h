#ifndef TERVEL_UTIL_PROGRESS_ASSURANCE_H_
#define TERVEL_UTIL_PROGRESS_ASSURANCE_H_

#include <atomic>
#include <memory>

#include "tervel/util/info.h"
#include "tervel/util/util.h"
#include "tervel/util/memory/hp/hp_element.h"


namespace tervel {
namespace util {

namespace memory {
namespace hp {
  class Element;
}
}

/**
 * This class is used to create Operation Records.
 * Operation records are designed to allow an arbitary thread to complete
 * some other thread's operation in the event that thread is unable to do so.
 * These objects are HP protected.
 */
// REVIEW(carlos): why is OpRecord a subclass of Element? shouldn't we keep
//   the two classes seperate? Otherwise, you're explicitly saying that the
//   progress assurance relies on hazard pointere'd memory (which is fine, but
//   if that's the case, progress assurance should be in a new namespace).
// RESPONSE(steven) Yes Progress assurance relies on hazard pointers to access
// elements in the optable. I moved it into the util name space, is that good?


class OpRecord : public memory::hp::Element {
 public:
  OpRecord() {}

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
  /**
   * Const used to delay an announcement
   */
  static constexpr size_t MAX_FAILURES = 1;

  /**
   * Const used to reduce the number of times a thread checks the table
   * Reduces memory loads at the cost of a higher upper bound
   */
  static constexpr size_t HELP_DELAY = 1;

  explicit ProgressAssurance(int num_threads)
      : op_table_(new std::atomic<OpRecord *>[num_threads] )
      , num_threads_ {num_threads} {}

  /**
   * This function checks at most one position in the op_table_ for an OPRecod
   * If one is found it will call its help_complete function.
   */
  static void check_for_announcement(ProgressAssurance *progress_assuarance =
          tervel::tl_thread_info->get_progress_assurance()) {
    progress_assuarance->p_check_for_announcement();
  }

  /**
   * This function places the
   * @param op an OpRecord to complete
   * @return on return the OpRecord must be completed.
   */
  static void make_announcement(OpRecord *op, int tid =
        tervel::tl_thread_info->get_thread_id(), ProgressAssurance *prog_assur =
        tervel::tl_thread_info->get_progress_assurance()) {
    prog_assur->p_make_announcement(op, tid);
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
  void p_make_announcement(OpRecord *op, int tid =
        tervel::tl_thread_info->get_thread_id());

  /**
   * Table for storing operation records, each thread has its own position
   * that corresponds to its thread if.
   */
  std::unique_ptr<std::atomic<OpRecord *>[]> op_table_;

  /**
   * The number of threads that are using this operation table
   */
  int num_threads_;

  DISALLOW_COPY_AND_ASSIGN(ProgressAssurance);
};

}  // namespace util
}  // namespace tervel

#endif  // TERVEL_UTIL_PROGRESS_ASSURANCE_H_
