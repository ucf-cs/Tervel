#include "mcas/mcas_helper.h"

namespace ucf {
namespace mcas {

bool MCASHelper::on_watch(std::atomic<void *> *address, T value) {
  int hp_pos = thread::hp::HazardPointer::SlotID::SHORTUSE;
  bool success = thread::hp::HazardPointer::watch(hp_pos, mcas_op_, address
                                                  , value);

  if (success) {
    /* Success, means that the MCAS object referenced by this MCH can not be
       freed while we check to make sure this MCH is assocaited with it. */
    t_MCASHelper *curr_mch = cas_row_->helper.load();
    if (curr_mch == nullptr) {
       if (cas_row_->helper.compare_exchange_strong(curr_mch, this)) {
         curr_mch = this;  // If this passed then curr_mch == nullptr, but we
                           //   need it to be == this
       }
    }
    if (curr_mch != this) {
      /* This MCH was placed in error, remove it and replace it with the
         logic value of this object (expected_value) */
      address->compare_exchange_strong(value, cas_row_->expected_value);
      success = false;
    }
  }  // End Successfull watch

  /* No longer need HP protection, if we have RC protection an associated MCH
     If we don't then the value at this address must have changed and 
     we don't need it either way */
  thread::hp::HazardPointer::unwatch(hp_pos);
  return success;
}


void * MCASHelper::complete(std::atomic<void *> *address, void * value) {
  t_MCASHelper* temp_null = nullptr;
  this->cas_row_->helper.compare_exchange_strong(temp_null, this);

  bool success = false;
  if (temp_null == nullptr || temp_null == this) {
    /* This implies it was successfully associated
       So call the complete function of the MCAS operation */
    success = t_MCAS::mcas_complete(this->mcas_op_, this->cas_row_);
    if (tl_thread_info->recursive_return) {
      /* If the thread is performing a recursive return back to its own operation
         Then just return null, it will be ignored. */
      return nullptr;
    }
    assert(this->last_row->helper.load() !=  nullptr);
  }

  if (success) {
  /* If the MCAS op was successfull then remove the MCH by replacing 
      it with the new_value */
    assert(this->last_row->helper.load() != t_MCAS::MCAS_FAIL_CONST);
    address->compare_exchange_strong(value,
        reinterpret_cast<void *>(this->cas_row_->new_value));
  } else { /*Otherwise remove the MCH by replacing it with the expected_value */
    address->compare_exchange_strong(value,
        reinterpret_cast<void *>(this->cas_row_->expected_value));
  }
  // Return the new current value of the position
  return address->load();
}

}  // End mcas namespace
}  // End ucf name space

