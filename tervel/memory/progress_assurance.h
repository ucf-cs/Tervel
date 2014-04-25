#ifndef TERVEL_MEMORY_PROGRESS_ASSURANCE_H_
#define TERVEL_MEMORY_PROGRESS_ASSURANCE_H_

#include <atomic>
#include <memory>

#include "tervel/memory/info.h"


namespace tervel {
namespace memory {
/**
 * This class is used to create Operation Records.
 * Operation records are designed to allow an arbitary thread to complete
 * some other thread's operation in the event that thread is unable to do so.
 * These objects are HP protected.
 */
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
  // Const used to reduce the number of times a thread checks the table
  // Reduces memory loads at the cost of a higher upper bound
  static constexpr size_t HELP_DELAY = 1;

  explicit ProgressAssurance(int num_threads =
                                      tl_thread_info.shared_info_->num_threads)
      : num_threads_ {num_threads}
      , op_table_(new std::atomic<OpRecord *>[num_threads_] ) {}

  /**
   * This function checks at most one position in the op_table_ for an OPRecod
   * If one is found it will call its help_complete function.
   */
  static void check_for_announcement(ProgressAssurance *progress_assuarance =
                      tl_thread_info.progress_assuarance) {
    progress_assuarance->p_check_for_announcement();
  }

  /**
   * This function places the
   * @param op an OpRecord to complete
   * @return on return the OpRecord must be completed.
   */
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

  void p_make_announcement(OpRecord *op, int tid = tl_thread_info.thread_id);

  /**
   * Table for storing operation records, each thread has its own position
   * that corresponds to its thread if.
   */
  std::unique_ptr<std::atomic<OpRecord *>[]> op_table_;
  // The number of threads that are using this operation table
  int num_threads_;
};

}  // namespace memory
}  // namespace tervel

#endif  // TERVEL_MEMORY_PROGRESS_ASSURANCE_H_
