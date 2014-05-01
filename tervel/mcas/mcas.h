#ifndef TERVEL_MCAS_MCAS_H_
#define TERVEL_MCAS_MCAS_H_

#include "tervel/mcas/mcas_helper.h"
#include "tervel/mcas/mcas_casrow.h"
#include "tervel/util/info.h"
#include "tervel/util/progress_assurance.h"
#include "tervel/util/memory/rc/descriptor_util.h"

#include <algorithm>
#include <cstdint>

namespace tervel {
namespace mcas {

template<class T>
// REVIEW(carlos): class does not follow class naming conventions
// RESPONSE(steven): Should it be MultiWordCompareAndSwap instead?
/**
 * This is the MCAS class, it is used to perform a Multi-Word Compare-and-Swap
 * To execute an MCAS, simply call addCASTriple for each address you want to
 * update, then call execute();
 * This function is wait-free.
 */
class MCAS : public util::OpRecord {
  typedef CasRow<T> t_CasRow;
  typedef Helper<T> t_Helper;

 private:
  enum class MCAS_STATE : std::int8_t {IN_PROGRESS = 0, PASS = 0 , FAIL = 0};

 public:
  static constexpr uintptr_t MCAS_FAIL_CONST = static_cast<uintptr_t>(0x1);

  explicit MCAS<T>(int max_rows)
        : cas_rows_(new t_CasRow[max_rows])
        , max_rows_ {max_rows} {}

  ~MCAS<T>() {
    for (int i = 0; i < row_count_; i++) {
      t_Helper* helper = cas_rows_[i].helper.load();
      // The No check flag is true because each was check prior
      // to the call of this descructor.
      util::memory::rc::free_descriptor(helper, true);
    }
  }

  /**
   * This function is used to add a CAS triple to the MCAS operation.
   * Each Triple consistens of an address to replace the expected value with
   * a new value iff each other address holds its expected value.
   *
   * This function returns false in the event the address already exists in
   * the mcas operation or the passed values are not valid. A Valid value
   * is a value which does not use one of the reserved bits or constants.
   * See related documentation for more details
   *
   * @param address
   * @param expected_value
   * @param new_value
   * @return true if successfully added.
   */
  bool add_cas_triple(std::atomic<T> *address, T expected_value, T new_value);

  /**
   * This function is called after all the CAS triples have been added to the
   * operation. It will attempt to apply the operation.
   * 
   * @returns true if it replaced the values at each address with a new value
   */
  bool execute();

 private:
  /** 
   * This function is used to complete a currently executing MCAS operation
   * It is most likely that this operation is in conflict with some other
   * operation and that it was discoved by the dereferencing of an MCH
   * The MCH contains a reference to the row in the MCAS it is associated with
   * and a reference to the final row in the mcas.
   * Using these two, we can complete the remaining rows.
   *
   * @param start_pos the row to begin at
   * @param wfmode if true it ignores the fail count because the operation
   * is in the op_table
   * @return whether or not the mcas succedded.
   */
  bool mcas_complete(int start_pos, bool wfmode = false);

  /** 
   * This function is used to cleanup a completed MCAS operation
   * It removes each MCH placed during this operation, replacing it with the
   * logical value
   */
  void cleanup(bool success);

  /**
   * This function insures that upon its return that *(cas_rows_[pos].address) no 
   * longer equals value. Where value is an object that holds an RC bit mark.
   * If the object is not a Helper for this operation, then the standard
   * descriptor remove function is called. This is important to prevent the case
   * where the stack increases beyond reasonable levels when multiple threads are
   * helping to complete the same operation.
   *
   * @param pos the cas_row which is blocked from completing its operaiton
   * @param value the value read at the position
   * @return the new current value
   **/
  T mcas_remove(const int pos, T value);

  void help_complete() {
    mcas_complete(0);
  }


  std::unique_ptr<t_CasRow[]> cas_rows_;
  std::atomic<MCAS_STATE> state_ {MCAS_STATE::IN_PROGRESS};
  int row_count_ {0};
  int max_rows_;
};  // MCAS class

template<class T>
bool MCAS<T>::add_cas_triple(std::atomic<T> *address, T expected_value,
      T new_value) {
  if (tervel::util::isValid(reinterpret_cast<void *>(expected_value)) ||
        tervel::util::isValid(reinterpret_cast<void *>(new_value))) {
    return false;
  } else if (row_count_ == max_rows_) {
    return false;
  } else {
    cas_rows_[row_count_].address_ = address;
    cas_rows_[row_count_].expected_value_ = expected_value;
    cas_rows_[row_count_].new_value_ = new_value;
    cas_rows_[row_count_++].helper_.store(nullptr);

    for (int i = (row_count_ - 1); i > 0; i--) {
      if (cas_rows_[i] > cas_rows_[i-1]) {
        swap(cas_rows_[i], cas_rows_[i-1]);
      } else if (cas_rows_[i] == cas_rows_[i-1]) {
        for (; i < row_count_-1; i++) {
          swap(cas_rows_[i], cas_rows_[i+1]);
        }
        row_count_--;

        /* We use reinterpret_cast in the event the specied data type is an
         * integer type.
         */
        cas_rows_[row_count_].address_ = nullptr;
        cas_rows_[row_count_].expected_value_ = reinterpret_cast<T>(nullptr);
        cas_rows_[row_count_].new_value_ = reinterpret_cast<T>(nullptr);
        return false;
      }
    }
    return true;
  }
}

template<class T>
bool MCAS<T>::execute() {
  tervel::util::ProgressAssurance::check_for_announcement();
  bool res = mcas_complete(0);
  cleanup(res);
  return res;
}


// REVIEW(carlos): This function is way too long. If it doesn't fit in ~1 screen
//   of code, I have trouble reading what it's doing. You should break this up
//   into auxiliary functions. You don't need to declare said functions in the
//   .h file, just put them into an unnamed namespace in the .cc file.
// REVIEW(carlos): TBH, the body of this function is impenatrable to me, and I
//   can't review the contents for correctness as-is.
// RESPONSE(steven): I added comments, but I am unsure how to divide it into
// sub functions.
template<class T>
bool MCAS<T>::mcas_complete(int start_pos, bool wfmode) {
  /**
   * Loop for each row in the op, if helping complete another thread's MCAS
   * Start at last known completed row.
   */
  for (int pos = start_pos; pos < row_count_; pos++) {
    size_t fcount = 0;  // Tracks the number of failures.

    t_CasRow * row = &cas_rows_[pos];

    /* Read the current value of the address */
    T current_value = row->address_->load();

    while (row->helper_.load() == nullptr) {
      /* Loop until this row's helper is no longer null */

      if (state_.load() != MCAS_STATE::IN_PROGRESS) {
        /* Checks if the operation has been completed */
        return (state_.load() == MCAS_STATE::PASS);
      } else if (!wfmode &&
              fcount++ == util::ProgressAssurance::MAX_FAILURES) {
        /* Check if we need to enter wf_mode */
        if (tervel::tl_thread_info->get_recursive_depth() == 0) {
          /* If this is our operation then make an annoucnement */
          tervel::util::ProgressAssurance::make_announcement(this);
          assert(state_.load() == MCAS_STATE::FAIL);
          return (state_.load() == MCAS_STATE::PASS);
        } else {
          /* Otherwise perform a recursive return */
          tervel::tl_thread_info->set_recursive_return();
          return false;
        }
      }

      /* Process the current value at the address */
      /* Now Check if the current value is descriptor */
      if (util::memory::rc::is_descriptor_first(
            reinterpret_cast<void *>(current_value))) {
        /* Remove it by completing the op, try again */
        current_value = this->mcas_remove(pos, current_value);

        /* Check if we are executing a recurisve return and if so determine
         * if we are at our own operation or need to return farther.
         */
        if (tervel::tl_thread_info->recursive_return()) {
          if (tervel::tl_thread_info->get_recursive_depth() == 0) {
            /* we are back to our own operation so re-read process the
             * current value
             */
            tervel::tl_thread_info->clear_recursive_return();
            current_value = row->address_->load();
            continue;
          } else {
            /* we need to return some more */
            return false;
          }
        }
        continue;
      } else if (current_value != row->expected_value_) {
        /* Current value does not match the expected value and it is a non 
         * descriptor type, the mcas operation should fail.
         */
        t_Helper* temp_null = nullptr;
        /* First try to disable row by assigning a failed constant */
        if (row->helper_.compare_exchange_strong(temp_null,
              reinterpret_cast<t_Helper *>(MCAS_FAIL_CONST))
              || temp_null == reinterpret_cast<t_Helper *>(MCAS_FAIL_CONST)) {
          /* if row was disabled then set the state to FAILED */
          MCAS_STATE temp_state = MCAS_STATE::IN_PROGRESS;
          this->state_.compare_exchange_strong(temp_state, MCAS_STATE::FAIL);
          assert(this->state_.load() == MCAS_STATE::FAIL);
          return false;
        } else {
          /* the row was associated, this implies the operation is over. We
           * will re-loop and observe the value of this->state_
           */
          continue;
        }
      }  else {
        /* Else the current_value matches the expected_value_ */
        t_Helper *helper = util::memory::rc::get_descriptor<t_Helper>(
              row, this);
        if (row->address_->compare_exchange_strong(current_value,
                util::memory::rc::mark_first(helper))) {
          /* helper was successfully placed at the address */
          t_Helper * temp_null = nullptr;
          if (row->helper_.compare_exchange_strong(temp_null, helper)
                || temp_null == helper) {
            /* We successfully associated the helper the row */
            break;  /* break the while loop, on to the next row! */
          } else {
            /* We failed to associate the helper, remove it, this implies that
             * the operation is over
             */
            T temp_helper = util::memory::rc::mark_first(helper);
            row->address_->compare_exchange_strong(temp_helper,
                  row->expected_value_);
            util::memory::rc::free_descriptor(helper);

            assert(state_.load() == MCAS_STATE::IN_PROGRESS);
            return (state_.load() == MCAS_STATE::PASS);
          }
        } else {
          /* We failed to place helper, re-evaluate the current_value
           * Set no check to true since it was never used.
           */
          util::memory::rc::free_descriptor(helper, true);
          continue;
        }
      }  // End Else Try to replace
    }  // End While Current helper is null

    if (row->helper_.load() == reinterpret_cast<t_Helper *>(MCAS_FAIL_CONST)) {
      MCAS_STATE temp_state = MCAS_STATE::IN_PROGRESS;
      this->state_.compare_exchange_strong(temp_state, MCAS_STATE::FAIL);
      assert(this->state_.load() == MCAS_STATE::FAIL);
      return false;
    }
  }  // End For Loop on CasRows

  /* All rows have been associated, so set the state to passed! */
  MCAS_STATE temp_state = MCAS_STATE::IN_PROGRESS;
  if (this->state_.compare_exchange_strong(temp_state, MCAS_STATE::PASS)) {
    temp_state = MCAS_STATE::PASS;
  }
  assert(this->state_.load() != MCAS_STATE::IN_PROGRESS);
  return (temp_state == MCAS_STATE::PASS);
}  // End Complete function.

template<class T>
T MCAS<T>::mcas_remove(const int pos, T value) {
  std::atomic<void *> *address = reinterpret_cast<std::atomic<void *>*>(
            cas_rows_[pos].address_);
    tervel::util::Descriptor *descr = util::memory::rc::unmark_first(
          reinterpret_cast<void *>(value));

  // First get a watch on the object.
  bool watched = util::memory::rc::watch(descr, address,
          reinterpret_cast<void *>(value));
  // Now unwatch it, crazy right? But there is a reason...
  util::memory::rc::unwatch(descr);
  if (watched) {
    /* If we watched it and it is a MCH for this operation, then act of 
     * watching will call the MCH's on_watch function, which will associate it
     * with this cas row...thus if cas_rows_[pos].helper != nullprt, then this
     * could have been a MCH to for this row but it does not matter, because
     * this row is already done, so we need to go to the next row.
     */
    if (this->cas_rows_[pos].helper_.load() != nullptr) {
      return reinterpret_cast<T>(nullptr);  // Does not matter, it wont be used.
    }
  } else {
    // watch failed do to the value at the address changing, return new value
    return reinterpret_cast<T>(address->load());
  }
}

template<class T>
void MCAS<T>::cleanup(bool success) {
  for (int pos = 0; pos < row_count_; pos++) {
    /* Loop for each row in the op*/
    t_CasRow * row = &cas_rows_[pos];

    assert(row->helper_.load() != nullptr);

    void * marked_helper = util::memory::rc::mark_first(row->helper_.load());
    if (marked_helper == reinterpret_cast<void *>(MCAS_FAIL_CONST)) {
      // There can not be any any associated rows beyond this position.
      return;
    } else {
      // else there was a helper placed for this row
      T cur_value = row->address_->load();
      if (cur_value == marked_helper) {
        if (success) {
          row->address_->compare_exchange_strong(cur_value, row->new_value_);
        } else {
          row->address_->compare_exchange_strong(cur_value,
                row->expected_value_);
        }
      }  // End If the current value matches the helper placed for this op
    }  // End Else there was a helper placed for this row
  }  // End While loop over casrows.
}  // End cleanup function.

}  // namespace mcas
}  // namespace tervel

#endif  // TERVEL_MCAS_MCAS_H_
