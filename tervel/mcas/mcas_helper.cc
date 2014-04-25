#include "mcas/mcas_helper.h"

namespace ucf {
namespace mcas {

bool MCASHelper::on_watch(std::atomic<void *> *address, T value) {
  int hp_pos = thread::hp::HazardPointer::SlotID::SHORTUSE;
  bool success = thread::hp::HazardPointer::watch(hp_pos, last_row, address, value);

  if (success) {
    /* Success, means that the MCAS object referenced by this MCH can not be
     * freed while we check to make sure this MCH is assocaited with it. */
    t_MCASHelper *curr_mch = cr->helper.load();
    if (curr_mch == nullptr) {
       if (cr->helper.compare_exchange_strong(curr_mch, this)) {
         curr_mch = this; //If this passed then curr_mch == nullptr, but we
                          // need it to be == this
       }
    }
    if (curr_mch != this) {
      /* This MCH was placed in error, remove it and replace it with the
         logic value of this object (expected_value) */
      address->compare_exchange_strong(value, cr->expected_value);
      success = false;
    } 

  }

  //No longer need HP protection, if we have RC portection an associated MCH
  //If we don't then the value at this address must have changed and 
  // we don't need it either way
  thread::hp::HazardPointer::unwatch(hp_pos);
  return success;
};


void * MCASHelper::complete(std::atomic<void *> *address, void * value) {
  t_MCASHelper* temp_null = nullptr;
  this->cr->helper.compare_exchange_strong(temp_null, this);

  bool success = false;
  if (temp_null == nullptr || temp_null == this) {
    //This implies it was successfully associated
    //So call the complete function of the MCAS operation
    success = t_MCAS::mcas_complete(this->cr, this->last_row);
    if (tl_thread_info->recursive_return) {
      //If the thread is performing a recursive return back to its own operation
      // Then just return null, it will be ignored.
      return nullptr;
    }
    assert(this->last_row->helper.load() !=  nullptr);
  }

  if (success) { //If the MCAS op was successfull
                 // Then remove the MCH by replacing it with the new_value
    assert(this->last_row->helper.load() != t_MCAS::MCAS_FAIL_CONST);
    address->compare_exchange_strong(value,
        reinterpret_cast<void *>(this->cr->new_value));
  } else { // Otherwise remove the MCH by replacing it with the expected_value
    address->compare_exchange_strong(value,
        reinterpret_cast<void *>(this->cr->expected_value));
  }
  //Return the new current value of the position
  return address->load();
}

T MCASHelper::mcas_remove(std::atomic<T> *address, T value, 
                            t_CasRow *last_row) {
  if (RC::is_descriptor_first(value)) {
    Descriptor *descr = RC::unmark_first(value);


    bool watched = thread::rc::watch(descr,
                  reinterpret_cast<std::atomic<void *>*>(address),
                  reinterpret_cast<void *>(t));
    if (!watched) {
      //watch failed do to the value at the address changing, return new value
      return address->load();
    }


    t_MCASHelper* cast_p = dynamic_cast<t_MCASHelper *>(descr);
    // TODO(steven): Hey carlos is there a betterway to check if this is a MCH
    // type?

    if ( (cast_p !=  nullptr) && (cast_p->last_row == last_row) ) {
      //This is a MCH for the same op, ie same position.
      assert((uintptr_t)cast_p == (uintptr_t)descr);

      //Make sure it is associated
      t_MCASHelper* temp_null = nullptr;
      cast_p->cr->helper.compare_exchange_strong(temp_null, cast_p);
      if (temp_null != nullptr && temp_null != cast_p) {
        address->compare_exchange_strong(t, cast_p->cr->expected_value);
      }
      
      thread::rc::unwatch(descr);
      return address->load();

    }
    thread::rc::unwatch(descr);  
  } 
  //Otherwise it is a non-mcas descriptor
  return reinterpret_cast<T>(thread::remove_descriptor(address, value));
  
};


}  // End mcas namespace
}  // End ucf name space

