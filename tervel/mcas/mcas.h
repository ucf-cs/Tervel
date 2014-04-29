// REVIEW(carlos): should be TERVEL_*
#ifndef UCF_MCAS_MCAS_H_
#define UCF_MCAS_MCAS_H_
// REVIEW(carlos): should be a space between block of includes and the include
//   guards.
// REVIEW(carlos): paths should start w/ tervel
#include "mcas/mcas_helper.h"
#include "mcas/mcas_casrow.h"


// REVIEW(carlos): should be namespace tervel
namespace ucf {
namespace mcas {

template<class T>
// REVIEW(carlos): class does not follow class naming conventions
// REVIEW(carlos): too many colons (class MCAS : public memory::OpRecord ...)
// REVIEW(carlos): You don't include the op record header, so this will cause an
//   unknown type error.
class MCAS : public : memory::OpRecord {
  /**
   * This is the MCAS class, it is used to perform a Multi-Word Compare-and-Swap
   * To execute an MCAS, simply call addCASTriple for each address you want to
   * update, then call execute();
   * This function is wait-free.
   */
  typedef CasRow<T> t_CasRow;
  typedef MCASHelper<T> t_MCASHelper;
  typedef MCAS<T> t_MCAS;

 private:
  enum class MCAS_STATE {IN_PROGRESS, PASS, FAIL};

 public:
  // REVIEW(carlos): wierd line breaking. next line should just be indented 2
  //   indentation levels (4 spaces)
  static constexpr t_MCASHelper * MCAS_FAIL_CONST =
                                        reinterpret_cast<t_MCASHelper *>(0x1);

  explicit MCAS<T>(int max_rows)
  // REVIEW(carlos): initializer list should be indented 4 spaces.
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
   * @param address
   * @param expected_value
   * @param new_value
   * @return true if successfully added.
   */
  // REVIEW(carlos):blank line should not exist

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

  // REVIEW(carlos): excess space at bottom of comment
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
  // REVIEW(carlos): shouldn't need to specify t_MCAS for this function.
  T t_MCAS::mcas_remove(const int pos, T value)


  // REVIEW(carlos): needs a trailing underscore
  t_CasRow[] cas_rows;

  std::atomic<MCAS_STATE> state_;
  int row_count_;
  int max_rows_;
// REVIEW(carlos): class is missing closing brace
// REVIEW(carlos): the closing comment should just repeat what's on the opening
//   line. Here, they should just say `namespace mcas' and `namespace ucf'
}  // End mcas namespace
// REVIEW(carlos): should be namespace tervel
}  // End ucf namespace

// REVIEW(carlos): should be TERVEL_*
#endif  // UCF_MCAS_MCAS_H_
