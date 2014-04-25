#include "tervel/mcas/mcas.h"
#include <algorithm>
namespace ucf {
namespace mcas {

bool t_MCAS::addCASTriple(std::atomic<T> *a, T ev, T nv) {
  if (memory::Descriptor::isValid(ev) ||  memory::Descriptor::isValid(nv)) {
    return false;
  } else if (row_count == max_rows_) {
    return false;
  } else {
    cas_rows[row_count].address = a;
    cas_rows[row_count].expected_value = ev;
    cas_rows[row_count].newValue = nv;
    cas_rows[row_count++].helper.store(nullptr);

    for (int i = (row_count - 1); i > 0; i--) {
      if (cas_rows[i] > cas_rows[i-1]) {
        swap(cas_rows[i], cas_rows[i-1]);
      } else if (cas_rows[i] == cas_rows[i-1]) {
        for (; i < row_count-1; i++) {
          swap(cas_rows[i], cas_rows[i+1]);
        }
        row_count--;
        cas_rows[row_count].address = reinterpret_cast<T>(nullptr);
        cas_rows[row_count].expected_value = reinterpret_cast<T>(nullptr);
        cas_rows[row_count].newValue = reinterpret_cast<T>(nullptr);
        return false;
      }
    }
    return true;
  }
};

bool t_MCAS::execute() {
  memory::ProgressAssurance::check_for_announcement();

  bool res = mcas_complete(0);

  // Now Clean up
  cleanup(res);
  return res;
};

bool t_MCAS::mcas_complete(int start_pos, bool wfmode = false) {
  for (int pos = start_pos; pos < row_count_; pos++) {
    /* Loop for each row in the op, if helping complete another thread's MCAS
       Start at last known completed row. */

    size_t fcount = 0;  // Tracks the number of failures.

    t_CasRow * row = &cas_rows[pos];
    T current_value = row->address->load();

    while (row->helper.load() == nullptr) {
      if (state_.load() != MCAS_STATE::IN_PROGRESS) {
        // Checks if the operation has been completed
        return (state_.load() == MCAS_STATE::PASS);
      } else if (!wfmode &&
                  fcount++ == memory::ProgressAssurance::MAX_FAILURE) {
        if (tl_thread_info.rDepth == 0) {
          // Make An annoucnement
          return this->wfcomplete();
        } else {
          tl_thread_info.recursive_return = true;
          return false;
        }
      }  // End else if Fail Count has been Reached

      if (memory::Descriptor::is_descriptor(current_value)) {
        // Remove it by completing the op, try again
        current_value = this->mcas_remove(pos, current_value);

        if (tl_thread_info.recursive_return) {
          if (tl_thread_info.rDepth == 0) {
            tl_thread_info.recursive_return = false;
            current_value = row->address->load();
          } else {
            return false;
          }
        }
        continue;
      } else if (current_value != row->expected_value) {
        // Current Value Does not match and it is a non descriptor type, so
        // The operation has failed.
        t_MCASHelper* temp_null = nullptr;
        if (row->helper.compare_exchange_strong(temp_null, MCAS_FAIL_CONST)
                                            || temp_null == MCAS_FAIL_CONST) {
          MCAS_STATE temp_state = MCAS_STATE::IN_PROGRESS;
          this->state_.compare_exchange_strong(temp_state, MCAS_STATE::FAIL);
          assert(this->stat_.load() == MCAS_STATE::FAIL);
          return false;
        }
        // Or perhaps not? Lets re-evaulte, the while condition must be false
        continue;
      }  else {
        // Elese the current_vale matches the expected_value
        t_MCASHelper *helper =
            memory::rc::DescriptorPool::get_descriptor<t_MCASHelper>(row, this);
        if (row->address->compare_exchange_strong(current_value,
                                memory::rc::mark_first(helper))) {
          // Succesfully placed
          t_MCASHelper * temp_null = nullptr;
          if (row->helper.compare_exchange_strong(temp_null, helper)
                                                || temp_null == helper) {
            // Succesfully associoated!
            break;  // On to the next row!
          } else {
            // Failed...op must be done!
            T temp_helper = memory::rc::mark_first(helper);
            row->address->compare_exchange_strong(temp_helper,
                                      row->expected_value);
            memory::rc::DescriptorPool::free_descriptor(helper);
          }
          break;
        } else {
          // Failed to place...try again!
          memory::rc::DescriptorPool::free_descriptor(helper, true);
          // Set no check to true since it was never used.
          continue;
        }
      }  // End Else Try to replace
    }  // End While Current helper is null
  }  // End For Loop on CasRows

  // Alll rows have been associated, so set the state to passed!
  MCAS_STATE temp_state = MCAS_STATE::IN_PROGRESS;
  if (this->state_.compare_exchange_strong(temp_state, MCAS_STATE::PASS)) {
    temp_state = MCAS_STATE::PASS;
  }
  assert(this->state_.load() != MCAS_STATE::IN_PROGRESS);
  return (temp_state == MCAS_STATE::PASS);
}  // End Complete function.

T t_MCAS::mcas_remove(const int pos, T value) {
  if (memory::rc::is_descriptor_first(value)) {
    // Checks if it is an rc_discriptor, could be another type (or bitmark)
    Descriptor *descr = memory::rc::unmark_first(value);

    // Gains RC protection on the object, if succesful then the object cant
    // be freed while we dereference it.
    bool watched = thread::rc::watch(descr,
                  reinterpret_cast<std::atomic<void *>*>(address),
                  reinterpret_cast<void *>(t));
    if (!watched) {
      // watch failed do to the value at the address changing, return new value
      return address->load();
    }


    t_MCASHelper* cast_p = dynamic_cast<t_MCASHelper *>(descr);
    // TODO(steven): Hey carlos is there a betterway to check if this is a MCH
    // type?

    if ( (cast_p !=  nullptr) && (cast_p->mcas_op_ == mcas_op_) ) {
      // This is a MCH for the same op, ie same position.
      assert((uintptr_t)cast_p == (uintptr_t)descr);
      assert(cast_p->cas_row_ == &(cas_row_[pos]));

      // Make sure it is associated
      t_MCASHelper* temp_null = nullptr;
      cas_row_[pos]->helper.compare_exchange_strong(temp_null, cast_p);
      if (temp_null != nullptr && temp_null != cast_p) {
        // Was placed in error/can't assoactie, so remove
        address->compare_exchange_strong(t, cast_p->cas_row_->expected_value);
      }
      thread::rc::unwatch(descr);
      return address->load();
    }
    thread::rc::unwatch(descr);
  }
  // Otherwise it is a non-mcas descas_row_iptor
  return reinterpret_cast<T>(memory::Descriptor::remove_descriptor
                                                              (address, value));
};



void cleanup(bool success) {
  for (int pos = 0; pos < row_count_; pos++) {
    /* Loop for each row in the op*/
    t_CasRow * row = &cas_rows[pos];

    assert(row->helper.load() != nullptr);
    void * marked_helper = memory::RC::mark_first(row->helper.load());
    if (marked_temp == reinterpret_cast<void *>(MCAS_FAIL_CONST)) {
      // There can not be any any associated rows beyond this position.
      return;
    } else {
      // else there was a helper placed for this row
      T cur_value = row->address->load();
      if (cur_value == marked_temp) {
        if (success) {
          row->address->compare_exchange_strong(cur_value, row->new_value);
        } else {
          row->address->compare_exchange_strong(cur_value, row->expected_value);
        }
      }  // End If the current value matches the helper placed for this op
    }  // End Else there was a helper placed for this row
  }  // End While loop over casrows.
}  // End cleanup function.


//-------
// Op Record functions
//-------

/**
 * This function is used to call the Progress Assurance Scheme
 * Upon its return it is guranteed to be completed.
 */
bool wf_complete() {
  tl_thread_info.progress_assurance->askForHelp(this);
  assert(this->state_.load() != MCAS_STATE::IN_PROGRESS);
  return (temp_state == MCAS_STATE::PASS);
}

void help_complete() {
  t_MCAS::mcas_complete(0, true);
};

}  // End mcas namespace
}  // End ucf name space
