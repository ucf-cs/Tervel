#include "tervel/mcas/mcas.h"
#include <algorithm>

bool MCAS::addCASTriple(std::atomic<T> *a, T ev, T nv) {
  if (thread::Descriptor::isValid(ev) ||  thread::Descriptor::isValid(nv)) {
    return false;
  } else if (row_count == max_rows_) {
    return false;
  } else {
    cas_rows[row_count].address = a;
    cas_rows[row_count].expectedValue = ev;
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
        cas_rows[row_count].expectedValue = reinterpret_cast<T>(nullptr);
        cas_rows[row_count].newValue = reinterpret_cast<T>(nullptr);
        return false;
      }
    }
    return true;
  }
};

bool MCAS::execute() {
  tl_thread_info->progreass_assurance->tryToHelp();
  if (row_count != max_rows_) {
    // TODO(steven): fix code to address when row_count < max_rows_
    // Also check if duplicate address...
    assert(false);
    return false;
  }

  bool res = mcas_complete(&(cas_rows[0]), &(cas_rows[row_count-1]));

  // Now Clean up
  cleanup(res, &(cas_rows[0]), &(cas_rows[row_count-1]));
  return res;
};

static bool MCAS::mcas_complete(t_CasRow *current, t_CasRow *last_row,
                                          bool wfmode = false) {
  current--;
  while (current != last_row) {
    current++;
    T temp = current->address->load();

    size_t fcount = 0;

    while (current->helper.load() == nullptr) {
      if (!wfmode && fcount++ == thread::OpRecord::MAX_FAILURE) {
        if (thread::rDepth == 0) {
          // Make An annoucnement
          return current->wfcomplete(last_row);
        } else {
          thread::rReturn = true;
          return false;
        }
      }  // End If Fail Count has been Reached

      if (RCDescr::isDescr(temp)) {  // Not should be replaced by isDescr?
        // Remove it by completing the op, try again
        temp = t_MCASHelper::mcasRemove(temp, current->address, last_row);

        if (thread::rReturn) {
          if (thread::rDepth == 0) {
            thread::rReturn = false;
            temp = current->address->load();
          } else {
            return false;
          }
        }
        continue;
      } else if (temp != current->expectedValue) {
        t_MCASHelper* temp_n = nullptr;
        if (current->helper.compare_exchange_strong(temp_n, MCAS_FAIL_CONST)
          || temp_n == MCAS_FAIL_CONST) {
          temp_n = nullptr;
          last_row->helper.compare_exchange_strong(temp_n, MCAS_FAIL_CONST);
          assert(last_row->helper.load() == MCAS_FAIL_CONST);
          return false;
        }
        continue;
      }  else {
        t_MCASHelper *helper = new t_MCASHelper(current, last_row);
        if (current->address->compare_exchange_strong(temp,
                                thread::Descriptor::rc_mark<T>(helper))) {
          // Succesfully placed
          t_MCASHelper * temp_null = nullptr;
          if (current->helper.compare_exchange_strong(temp_null, helper)
                                                || temp_null == helper) {
            // Succesfully associoated!
          } else {
            // Failed...op must be done!
            temp = thread::Descriptor::rc_mark<T>(helper);
            current->address->compare_exchange_strong(temp,
                                      current->expectedValue);
            helper->safeFree();
          }
          break;
        } else {
          // Failed to place...try again!
          // temp already holds the new value
          helper->unsafeFree();
          continue;
        }
      }  // End Else Try to replace
    }  // End loop on CasRow

    assert(current->helper.load() != nullptr);
    if (current->helper.load() == MCAS_FAIL_CONST) {
      t_MCASHelper* temp_n = nullptr;
      last_row->helper.compare_exchange_strong(temp_n, MCAS_FAIL_CONST);
      assert(last_row->helper.load() == MCAS_FAIL_CONST);
      return false;
    }
  }  // End While current != last_row

  assert(last_row->helper.load() != nullptr);
  return (last_row->helper.load() != MCAS_FAIL_CONST);
}  // End Complete function.

static void cleanup(bool success, t_CasRow *current, t_CasRow *last_row) {
  current--;
  while (current != last_row) {
    current++;

    assert(current->helper.load() != nullptr);
    T marked_temp = thread::Descriptor::rc_mark<T>(current->helper.load());
    if (marked_temp == reinterpret_cast<T>(MCAS_FAIL_CONST)) {
      return;
    }

    T temp_current = current->address->load();

    if (temp_current == marked_temp) {
      if (success) {
        current->address->compare_exchange_strong(temp_current,
                                                  current->newValue);
      } else {
        current->address->compare_exchange_strong(temp_current,
                                                  current->expectedValue);
      }
    }
  }  // End While loop over casrows.
}  // End cleanup function.
