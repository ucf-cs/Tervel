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
#ifndef TERVEL_MCAS_MCAS_H_
#define TERVEL_MCAS_MCAS_H_

#include <tervel/algorithms/wf/mcas/mcas_helper.h>
#include <tervel/algorithms/wf/mcas/mcas_casrow.h>
#include <tervel/util/info.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/rc/descriptor_util.h>
#include <cstdint>
#include <algorithm>

namespace tervel {
namespace algorithms {
namespace wf {
namespace mcas {
/**
 * This function determines the logical value of the address.
 *
 * @param address the location to determine the value
 * @return the logical value
 */
template<class T>
inline T read(std::atomic<T> *address) {
  void *value = util::memory::rc::descriptor_read_first(address);
  return reinterpret_cast<T>(value);
}


// REVIEW(carlos): class does not follow class naming conventions
// RESPONSE(steven): Should it be MultiWordCompareAndSwap instead?
/**
 * MultiWordCompareAndSwap class is used to perform a Multi-Word
 * Compare-and-Swap (MCAS)
 * To execute an MCAS, call addCASTriple for each address you want to update,
 * then call execute();
 *
 * This function is wait-free.
 */
template<class T>
class MultiWordCompareAndSwap : public util::OpRecord {
 public:
  static constexpr void * MCAS_FAIL_CONST = reinterpret_cast<void *>(0x1L);

  explicit MultiWordCompareAndSwap<T>(int max_rows)
      : cas_rows_(new CasRow<T>[max_rows])
      , max_rows_ {max_rows} {}

  ~MCAS<T>() {
    state_.store(MCasState::DELETED);
    for (int i = 0; i < row_count_; i++) {
      Helper<T>* helper = cas_rows_[i].helper_.load();
      // The No check flag is true because each was check prior
      // to the call of this descructor.
      if (helper == reinterpret_cast<Helper<T> *>(MCAS_FAIL_CONST)) {
        break;
      }
      util::memory::rc::free_descriptor(helper, true);
    }
  }

  /**
   * This function is used to add a CAS triple to the MCAS operation.
   * Each Triple consistent of an address to replace the expected value with
   * a new value iff each other address holds its expected value.
   *
   * This function returns false in the event the address already exists in
   * the MCAS operation or the passed values are not valid. A Valid value
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

  /**
   * THis function overrides the virtual function in the OpRecord class
   * It is called by the progress assurance scheme. Upon its return the MCAS
   * operation must be completed
   */
  void help_complete() {
    mcas_complete(0, true);
  }

  /**
   * This function overrides the virtual function in the HP::Element class
   * It returns whether or not this mcas class is being referenced by another
   * threaded. It is being referenced if any associated descriptor has a
   * positive reference count or if there is a hazard point watch on it.
   *
   * @return True if watched
   */
  bool on_is_watched() {
    for (int i = 0; i < row_count_; i++) {
      Helper<T>* helper = cas_rows_[i].helper_.load();
      // The No check flag is true because each was check prior
      // to the call of this destructor.
      if (helper == reinterpret_cast<Helper<T> *>(MCAS_FAIL_CONST)) {
        break;
      } else if (util::memory::rc::is_watched(helper)) {
        return true;
      }
    }
    return false;
  }

 private:
  /**
   * This enum is used to indicate the state of an mcas operation
   * DELETED is used in debugging procedures
   */
  enum class MCasState : std::int8_t {IN_PROGRESS = 0, PASS = 1 , FAIL = 2,
      DELETED = 3};
  /**
   * This function is used to complete a currently executing MCAS operation
   * It is most likely that this operation is in conflict with some other
   * operation and that it was discover by the dereferencing of an MCH
   * The MCH contains a reference to the row in the MCAS it is associated with
   * and a reference to the final row in the mcas.
   * Using these two, we can complete the remaining rows.
   *
   * @param start_pos the row to begin at
   * @param wfmode if true it ignores the fail count because the operation
   * is in the op_table
   * @return whether or not the mcas succeeded.
   */
  bool mcas_complete(int start_pos, bool wfmode = false);

  /**
   * Same as above, but it calculates the start_pos based on the current row
   * then calls the above mcas_complete.
   *
   * @param current_row the last known completed row
   * @return where or not the mcas succeeded
   */
  bool mcas_complete(CasRow<T> *current_row);

  /**
   * This function is used to cleanup a completed MCAS operation
   * It removes each MCH placed during this operation, replacing it with the
   * logical value
   */
  void cleanup(bool success);

  /**
   * This function insures that upon its return that *(cas_rows_[pos].address)
   * no longer equals value. Where value is an object that holds an RC bit mark.
   * If the object is not a Helper for this operation, then the standard
   * descriptor remove function is called. This is important to prevent the case
   * where the stack increases beyond reasonable levels when multiple threads
   * are helping to complete the same operation.
   *
   * @param pos the cas_row which is blocked from completing its operation
   * @param value the value read at the position
   * @return the new current value
   **/
  T mcas_remove(const int pos, T value);

  // ------
  // Member Data
  // ------

  /* THe array of CAS triples to complete*/
  std::unique_ptr<CasRow<T>[]> cas_rows_;
  /* The state of the mcas operation */
  std::atomic<MCasState> state_ {MCasState::IN_PROGRESS};
  /* The number of cas rows */
  int row_count_ {0};
  /* The max number of rows supported this instance is expecting*/
  int max_rows_;

  friend Helper<T>;
  friend CasRow<T>;
};  // MCAS class


}  // namespace mcas
}  // namespace wf
}  // namespace algorithms
}  // namespace tervel

#include <tervel/algorithms/wf/mcas/mcas_imp.h>
#endif  // TERVEL_MCAS_MCAS_H_
