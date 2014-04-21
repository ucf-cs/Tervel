#include "mcas/mcas_helper.h"

namespace ucf {
namespace mcas {

bool MCASHelper::advance_watch(std::atomic<void *> *address, T value) {
  int hp_pos = thread::hp::HazardPointer::slot_id::id_temp;
  tl_thread_info.hazard_pointer->watch(hp_pos,
                                       reinterpret_cast<void *>(last_row));

  if (address->load() == value) {
    // Alright, now both this helper can not be changed
    // and the mcas it reference can not be changed.
    // Lets insure they are associated

    t_MCASHelper *new_current = cr->helper.load();
    if (new_current == nullptr) {
       if (cr->helper.compare_exchange_strong(new_current, this)) {
         new_current = this;
       }
    }
    if (new_current != this) {
      // We have a hp proctection only, because it is not associated!
        // --ie placed in error, op is already done...
        // Remove it!
      T marked_helper = RCDescr::mark<T>(marked_helper);
      address->compare_exchange_strong(marked_helper, cr->expectedValue);
    }
  }

  tl_thread_info.hazard_pointer->unwatch(hp_pos);
  return (new_current == this);
};


void * MCASHelper::complete(void * v, std::atomic<void *> *address) {
  t_MCASHelper* temp_null = nullptr;
  this->cr->helper.compare_exchange_strong(temp_null, this);

  bool success = false;
  if (temp_null == nullptr || temp_null == this) {
    assert(this->cr->helper.load() != nullptr);
    assert(this->cr->helper.load() != t_MCAS::MCAS_FAIL_CONST);
    success = t_MCAS::mcas_complete(this->cr, this->last_row);
    if (thread::rReturn) {
      return nullptr;
    }
    assert(this->last_row->helper.load() !=  nullptr);
  }

  if (success) {
    assert(this->last_row->helper.load() != t_MCAS::MCAS_FAIL_CONST);
    address->compare_exchange_strong(v,
        reinterpret_cast<void *>(this->cr->newValue));
  } else {
    address->compare_exchange_strong(v,
        reinterpret_cast<void *>(this->cr->expectedValue));
  }
  return address->load();
}

T MCASHelper::mcas_remove(T t, std::atomic<T> *address,
                      t_CasRow *last_row) {
  RCDescr *descr = Descriptor::unmark(t);
  bool watched = thread::rc::PoolElem::watch(descr,
                reinterpret_cast<std::atomic<void *>*>(address),
                reinterpret_cast<void *>(t));

  T newValue;
  if (watched) {
    t_MCASHelper* cast_p = static_cast<t_MCASHelper *>(descr);
    // TODO(steven): check to make sure this is safe...
    if ( (cast_p !=  nullptr) && (cast_p->last_row == last_row) ) {
      assert((uintptr_t)cast_p == (uintptr_t)descr);

      t_MCASHelper* temp_null = nullptr;
      cast_p->cr->helper.compare_exchange_strong(temp_null, cast_p);
      if (temp_null != nullptr && temp_null != cast_p) {
        address->compare_exchange_strong(t, cast_p->cr->expectedValue);
      }
      newValue = address->load();
    } else {
      newValue = RCDescr::remove(t, address);
    }
    thread::rc::PoolElem::unwatch(descr);
  } else {
    newValue = address->load();
  }

  return newValue;
};


}  // End mcas namespace
}  // End ucf name space

