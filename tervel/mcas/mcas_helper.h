#ifndef TERVEL_MCAS_MCAS_HELPER_H_
#define TERVEL_MCAS_MCAS_HELPER_H_

#include "tervel/mcas/mcas.h"
#include "tervel/mcas/mcas_casrow.h"

#include "tervel/util/info.h"
#include "tervel/util/descriptor.h"
#include "tervel/util/memory/hp/hazard_pointer.h"

#include <atomic>

namespace tervel {
namespace mcas {

template<class T>
/**
 * This class is the MCAS operation's helper. The Helper or MCH is used to 
 * replace the expected value of the specified address. 
 */
class Helper : public util::Descriptor {
typedef CasRow<T> t_CasRow;
typedef Helper<T> t_Helper;
typedef MCAS<T> t_MCAS;

 public:
  /**
   * @param mcas_op the t_MCAS which contains the referenced cas_row
   * @param cas_row the referenced row in the t_MCAS.
   */
  Helper<T>(t_MCAS *mcas_op, t_CasRow *cas_row)
    : cas_row_(cas_row), mcas_op_(mcas_op) {}

  /**
   * This function is called after this objects rc count was incremented.
   * It acquires a temporary HP watch on the MCAS op (via last_row_), ensures
   * it is is associated, and if so returns true.
   * If it is not associated or it was removed, it returns false
   *
   * @param address the address this MCH was read from
   * @param value the bitmarked value of this MCH
   * @return returns whether or not the watch was successfull.
   *
   * Defined in Descriptor.h
   */
  // bool on_watch(std::atomic<void *> *address, void *value);

  /** 
   * This function is called to remove an MCH by completing its associated
   * operation.
   *
   * @params address the location where the MCH resides
   * @params value the value of this MCH as it was read at the address
   * @return the current value of the address
   *
   * Defined in Descriptor.h
   */
  // void * complete(std::atomic<void *> *address, void * value);

  /** 
   * This function is called to remove a descriptor which is in conflict with
   * a MCAS operation. If the descriptor is a MCH, then a check is performed
   * to determine if it is for the same operation. If so, then it is treated 
   * differently. Otherwise, the descriptors complete function is called.
   *
   * @params address the location where a descriptor was read from
   * @params value the value that was read which is a descriptor
   * @params last_row an identifier of the current MCAS operation 
   * @return the current value of the address
   */
  static T mcas_remove(std::atomic<T> *address, T value, t_CasRow *last_row);

  /**
   * Attempts to complete acquiring a memory watch on a Helper object
   * Declared in util/Descriptor.h
   * @param  address the location value was read form
   * @param  value the last known value of address
   * @return true if succesful acquiring the watch
   */

  bool on_watch(std::atomic<void *> *address, void * value) {
    typedef util::memory::hp::HazardPointer::SlotID t_SlotID;
    bool success = util::memory::hp::HazardPointer::watch(
            t_SlotID::SHORTUSE, mcas_op_, address, value);

    if (success) {
      /* Success, means that the MCAS object referenced by this Helper can not
       * be freed while we check to make sure this Helper is assocaited with
       * it.
       */
      t_Helper *curr_mch = cas_row_->helper.load();
      if (curr_mch == nullptr) {
         if (cas_row_->helper.compare_exchange_strong(curr_mch, this)) {
           /* If this passed then curr_mch == nullptr, so we set it to be == this
            */
            curr_mch = this;
         }
      }
      if (curr_mch != this) {
        /* This Helper was placed in error, remove it and replace it with the
         * logic value of this object (expected_value)
         */
        address->compare_exchange_strong(value, cas_row_->expected_value);
        success = false;
      }
    }  // End Successfull watch

    /* No longer need HP protection, if we have RC protection on an associated
     * Helper. If we don't it, the value at this address must have changed and 
     * we don't need it either way.
     */
    util::memory::hp::HazardPointer::unwatch(t_SlotID::SHORTUSE);
    return success;
  }

  void * complete(void *value, std::atomic<void *> *address) {
    t_Helper* temp_null = nullptr;
    this->cas_row_->helper.compare_exchange_strong(temp_null, this);

    bool success = false;
    if (temp_null == nullptr || temp_null == this) {
      /* This implies it was successfully associated
         So call the complete function of the MCAS operation */
      success = t_MCAS::mcas_complete(this->mcas_op_, this->cas_row_);
      if (tervel::tl_thread_info->recursive_return()) {
        /* If the thread is performing a recursive return back to its own operation
           Then just return null, it will be ignored. */
        return nullptr;
      }
      assert(this->last_row->helper.load() !=  nullptr);
    }

    if (success) {
    /* If the MCAS op was successfull then remove the Helper by replacing 
        it with the new_value */
      assert(this->last_row->helper.load() != t_MCAS::MCAS_FAIL_CONST);
      address->compare_exchange_strong(value,
          reinterpret_cast<void *>(this->cas_row_->new_value_));
    } else {
      /*Otherwise remove the Helper by replacing it with the expected_value*/
      address->compare_exchange_strong(value,
          reinterpret_cast<void *>(this->cas_row_->expected_value_));
    }
    // Return the new current value of the position
    return address->load();
  }

 private:
  // The Row in the MCAS operation this MCH was placed for
  t_CasRow *cas_row_;
  // The MCAS which contains the cas_row_
  t_MCAS *mcas_op_;
};  // Helper

}  // namespace mcas
}  // namespace tervel

#endif  // TERVEL_MCAS_MCAS_HELPER_H_
