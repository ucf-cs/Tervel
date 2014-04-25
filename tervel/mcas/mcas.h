#ifndef UCF_MCAS_MCAS_H_
#define UCF_MCAS_MCAS_H_
#include "mcas/mcas_helper.h"
#include "mcas/mcas_casrow.h"


namespace ucf {
namespace mcas {

template<class T>
class MCAS : public : memory::OpRecord {
  typedef CasRow<T> t_CasRow;
  typedef MCASHelper<T> t_MCASHelper;
  typedef MCAS<T> t_MCAS;

 private:
  enum class MCAS_STATE {IN_PROGRESS, PASS, FAIL};

 public:
  static constexpr t_MCASHelper * MCAS_FAIL_CONST =
                                        reinterpret_cast<t_MCASHelper *>(0x1);

  explicit MCAS<T>(int max_rows)
  : max_rows_ {max_rows}
  , row_count_ {0}
  , cas_rows {new t_CasRow[max_rows]}
  , state_ {MCAS_STATE::IN_PROGRESS} {}

  ~MCAS<T>() {
    for (int i = 0; i < row_count_; i++) {
      t_MCASHelper * mch = cas_rows[i].helper.load();
      // The No check flag is true because each was check prior
      // to the call of this descructor.
      memory::rc::DescriptorPool::free_descriptor(helper, true);
    }
    delete[] cas_rows;
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
   * @params address
   * @params expected_value
   * @params new_value
   * returns true if successfully added.
   */

  bool addCASTriple(std::atomic<T> *address, T expected_value, T new_value);

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
   * @param current is the last known completed CasRow
   * @param wfmode if true it ignores the fail count because the operation
   * is in the op_table
   * @return whether or not the mcas succedded.
   */
  bool mcas_complete(t_CasRow *current, bool wfmode = false);

  /** 
   * This function is used to cleanup a completed MCAS operation
   * It removes each MCH placed during this operation, replacing it with the
   * logical value
   *
   */
  void cleanup();

  /**
   * This function insures that upon its return that *(cas_rows[pos].address) no 
   * longer equals value. Where value is an object that holds an RC bit mark.
   * If the object is not a MCASHelper for this operation, then the standard
   * descriptor remove function is called. This is important to prevent the case
   * where the stack increases beyond reasonable levels when multiple threads are
   * helping to complete the same operation.
   *
   * @param pos the cas_row which is blocked from completing its operaiton
   * @param value the value read at the position
   * @return the new current value
   **/
  T t_MCAS::mcas_remove(const int pos, T value)


  t_CasRow[] cas_rows;

  std::atomic<MCAS_STATE> state_;
  int row_count_;
  int max_rows_;
}  // End mcas namespace
}  // End ucf namespace

#endif  // UCF_MCAS_MCAS_H_
