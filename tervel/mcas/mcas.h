#ifndef UCF_MCAS_MCAS_H_
#define UCF_MCAS_MCAS_H_
#include "mcas/mcas_helper.h"
#include "mcas/mcas_casrow.h"


namespace ucf {
namespace mcas {

  template<class T>
  class MCAS {
    typedef CasRow<T> t_CasRow;
    typedef MCASHelper<T> t_MCASHelper;
    typedef MCAS<T> t_MCAS;

  public:
    static constexpr t_MCASHelper * MCAS_FAIL_CONST =
                 reinterpret_cast<t_MCASHelper *>(0x1);
    t_CasRow *cas_rows;

    int row_count_;
    int max_rows_;
    explicit MCAS<T>(int max_rows)
    : max_rows_ {max_rows},
      row_count {0},
      cas_rows {new t_CasRow[max_rows]} {}

    ~MCAS<T>() {
      if (row_count == max_rows_) {
        t_CasRow *last_row = &(cas_rows[row_count-1]);
        last_row->safeFree();
        // Do not delete cas_rows, this will be done in safeFree
      } else {
        assert(false);
        delete[] cas_rows;
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

    /** 
     * This function is used to complete a currently executing MCAS operation
     * It is most likely that this operation is in conflict with some other
     * operation and that it was discoved by the dereferencing of an MCH
     * The MCH contains a reference to the row in the MCAS it is associated with
     * and a reference to the final row in the mcas.
     * Using these two, we can complete the remaining rows.
     *
     * @param current is the last known completed CasRow
     * @param last_row is the last_row of the operation
     * @param wfmode if true it ignores the fail count because the operation
     * is in the op_table
     * @return whether or not the mcas succedded.
     */
    static bool mcas_complete(t_CasRow *current, t_CasRow *last_row,
                                          bool wfmode = false);

    /** 
     * This function is used to cleanup a completed MCAS operation
     * TODO(steven) convert this into a member function
     */
    static void cleanup(bool success, t_CasRow *current, t_CasRow *last_row);

}  // End mcas namespace
}  // End ucf namespace

#endif  // UCF_MCAS_MCAS_H_
