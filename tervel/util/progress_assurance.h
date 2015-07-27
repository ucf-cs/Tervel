/*
The MIT License (MIT)

Copyright (c) 2015 University of Central Florida's Computer Software Engineering
Scalable & Secure Systems (CSE - S3) Lab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef TERVEL_UTIL_PROGRESS_ASSURANCE_H_
#define TERVEL_UTIL_PROGRESS_ASSURANCE_H_

#include <atomic>
#include <memory>
#include <assert.h>
#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/tervel_metrics.h>

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

  /**
   * This function is used to achieve a strong watch on an Element.
   * Classes wishing to express this should override this function.
   *
   * @param address the expected was load from
   * @param expected the last known value of address
   * @return whether or not the element was succefully watched.
   */
  using memory::hp::Element::on_watch;
  virtual bool on_watch(std::atomic<void *> *address, void *expected) {
    return true;
  }

  /**
   * This function is used to check a strong watch on an Element.
   * Classes wishing to express this should override this function.
   *
   * @return whether or not the element is watched.
   */
  using memory::hp::Element::on_is_watched;
  virtual bool on_is_watched() { return false; }
  /**
   * This function is used to remove a strong watch on an Element.
   * Classes wishing to express this should override this function.
   */
  using memory::hp::Element::on_unwatch;
  virtual void on_unwatch() {}


 private:
  DISALLOW_COPY_AND_ASSIGN(OpRecord);
};


/**
 * This class represents the progress assurance scheme employed by our library.
 * For this scheme to be effective, each operation which may indefinitely prevent
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
  //static constexpr size_t MAX_FAILURES = TERVEL_PROG_ASSUR_LIMIT;

  /**
   * Const used to reduce the number of times a thread checks the table
   * Reduces memory loads at the cost of a higher upper bound
   */
  static constexpr int64_t HELP_DELAY = TERVEL_PROG_ASSUR_DELAY;


  class Limit {
   public:
    explicit Limit(int64_t limit = TERVEL_PROG_ASSUR_LIMIT)
      : counter_(limit) {}

    bool notDelayed(int64_t val = 1) {
      return !isDelayed(val);
    }
    bool isDelayed(int64_t val = 1) {
      int64_t temp = counter_;
      counter_ -= val;
      return (temp == 0);
    }

    void reset(int64_t limit = TERVEL_PROG_ASSUR_LIMIT) {
      counter_ = limit;
    }
   private:
    int64_t counter_;
  };

  explicit ProgressAssurance(int64_t num_threads)
      : op_table_(new std::atomic<OpRecord *>[num_threads]() )
      , num_threads_ {num_threads} {}

  /**
   * This function checks at most one position in the op_table_ for an OPRecod
   * If one is found it will call its help_complete function.
   * help_id_ is a variable used to track which thread to check for an
   * announcement.

   * delay_count_ is a variable used to delay how often a thread checks for an
   * annoucnement
  */
  static void check_for_announcement(ProgressAssurance * const progress_assuarance =
        nullptr) {
    static __thread int64_t delay_count = HELP_DELAY;
    static __thread int64_t help_id = 0;

    if (delay_count-- == 0) {
      delay_count = HELP_DELAY;
      if (progress_assuarance ==  nullptr) {
        tervel::tl_thread_info->get_progress_assurance()->
          p_check_for_announcement(help_id);
      } else {
        progress_assuarance->p_check_for_announcement(help_id);
      }
    }
  }

  /**
   * This function places the
   * @param op an OpRecord to complete
   * @return on return the OpRecord must be completed.
   */
  static void make_announcement(OpRecord *op, const uint64_t tid =
        tervel::tl_thread_info->get_thread_id(), ProgressAssurance * const prog_assur =
        tervel::tl_thread_info->get_progress_assurance()) {
    util::EventTracker::countEvent(util::EventTracker::event_code::announcement);

    prog_assur->p_make_announcement(op, tid);
  }

 private:
  /**
   * This function checks at most one position in the op_table_ for an OPRecod
   * If one is found it will call its help_complete function.
   */
  void p_check_for_announcement(int64_t &hpos);

  /**
   * This function places the
   * @param op an OpRecord to complete
   * @return on return the OpRecord must be completed.
   */
  void p_make_announcement(OpRecord *op, const uint64_t tid =
        tervel::tl_thread_info->get_thread_id());

  /**
   * Table for storing operation records, each thread has its own position
   * that corresponds to its thread if.
   */
  std::unique_ptr<std::atomic<OpRecord *>[]> op_table_;

  /**
   * The number of threads that are using this operation table
   */
  const int64_t num_threads_;

  DISALLOW_COPY_AND_ASSIGN(ProgressAssurance);
};

}  // namespace util
}  // namespace tervel

#endif  // TERVEL_UTIL_PROGRESS_ASSURANCE_H_
