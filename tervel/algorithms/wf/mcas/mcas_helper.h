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
#ifndef TERVEL_MultiWordCompareAndSwap_MultiWordCompareAndSwap_HELPER_H_
#define TERVEL_MultiWordCompareAndSwap_MultiWordCompareAndSwap_HELPER_H_

#include <tervel/util/info.h>
#include <tervel/util/descriptor.h>
#include <tervel/util/memory/hp/hazard_pointer.h>
#include <tervel/util/memory/rc/descriptor_util.h>

#include <tervel/algorithms/wf/mcas/mcas.h>
#include <tervel/algorithms/wf/mcas/mcas_casrow.h>

namespace tervel {
namespace algorithms {
namespace wf {
namespace mcas {

template<class T>
class MultiWordCompareAndSwap;
template<class T>
class CasRow;

template<class T>
/**
 * This class is the MultiWordCompareAndSwap operation's helper. The Helper or MCH is used to
 * replace the expected value of the specified address.
 */
class Helper : public util::Descriptor {
 public:
  /**
   * The Helper object contains a reference to the row it is associated with and
   * the mcas operation that contains the row.
   *
   * @param mcas_op the MultiWordCompareAndSwap<T> which contains the referenced cas_row
   * @param cas_row the referenced row in the MultiWordCompareAndSwap<T>.
   */
  Helper<T>(MultiWordCompareAndSwap<T> *mcas_op, CasRow<T> *cas_row)
    : cas_row_(cas_row), mcas_op_(mcas_op) {}

  /**
   * This function is called after this objects rc count was incremented.
   * It acquires a temporary HP watch on the MultiWordCompareAndSwap op (via last_row_), ensures
   * it is is associated, and if so returns true.
   * If it is not associated or it was removed, it returns false
   *
   * @param address the address this MCH was read from
   * @param value the bitmarked value of this MCH
   * @return returns whether or not the watch was successfull.
   */
  using util::Descriptor::on_watch;
  bool on_watch(std::atomic<void *> *address, void * value) {
    typedef util::memory::hp::HazardPointer::SlotID t_SlotID;
    bool success = util::memory::hp::HazardPointer::watch(
          t_SlotID::SHORTUSE, mcas_op_, address, value);

    if (success) {
      /* Success, means that the MultiWordCompareAndSwap object referenced by this Helper can not
       * be freed while we check to make sure this Helper is assocaited with
       * it. */
      Helper<T> *curr_mch = cas_row_->helper_.load();
      if (curr_mch == nullptr) {
         if (cas_row_->helper_.compare_exchange_strong(curr_mch, this)) {
           /* If this passed then curr_mch == nullptr, so we set it to be ==
            * this */
            curr_mch = this;
         }
      }

      assert(cas_row_->helper_.load() == curr_mch);

      if (curr_mch != this) {
        /* This Helper was placed in error, remove it and replace it with the
         * logic value of this object (expected_value)
         */
        address->compare_exchange_strong(value, cas_row_->expected_value_);
        success = false;
      }
      /* No longer need HP protection, if we have RC protection on an associated
       * Helper. If we don't it, the value at this address must have changed and
       * we don't need it either way.
       */
      util::memory::hp::HazardPointer::unwatch(t_SlotID::SHORTUSE);
    }  // End Successfull watch

    if (success) {
      assert(cas_row_->helper_.load() != nullptr);
      assert(util::memory::rc::is_watched(this));
      assert(util::memory::hp::HazardPointer::is_watched(mcas_op_));
    }

    return success;
  };


  /**
   * This function completes the mcas operation associated with this helper.
   * before returning to removes the helper and identifies the new current value
   * of the address
   * @param value the last known current value at address
   * @param address the location value was read from
   * @return the new current value of the address
   */
  using util::Descriptor::complete;
  void * complete(void *value, std::atomic<void *> *address) {
    assert(cas_row_->helper_.load());
    assert(util::memory::rc::is_watched(this));
    assert(util::memory::hp::HazardPointer::is_watched(this->mcas_op_));
    Helper<T>* temp_null = nullptr;
    this->cas_row_->helper_.compare_exchange_strong(temp_null, this);

    bool success = false;
    if (temp_null == nullptr || temp_null == this) {
      /* This implies it was successfully associated
         So call the complete function of the MultiWordCompareAndSwap operation */
      success = this->mcas_op_->mcas_complete(this->cas_row_);
      if (util::RecursiveAction::recursive_return()) {
        /* If the thread is performing a recursive return back to its own
           operation, then just return null, it will be ignored. */
        return nullptr;
      }
    }

    if (success) {
    /* If the MultiWordCompareAndSwap op was successfull then remove the Helper by replacing
        it with the new_value */
      address->compare_exchange_strong(value,
          reinterpret_cast<void *>(this->cas_row_->new_value_));
    } else {
      /*Otherwise remove the Helper by replacing it with the expected_value*/
      address->compare_exchange_strong(value,
          reinterpret_cast<void *>(this->cas_row_->expected_value_));
    }
    // Return the new current value of the position
    return address->load();
  };

  /**
   * This function is only called on helpers which are associated with its
   * mcas operation, if it is not, then the call to on_watch would have failed
   * and this would not have been called.
   *
   * @return the logicial value of this descriptor object.
   */
  using util::Descriptor::get_logical_value;
  void * get_logical_value() {
    if (this->mcas_op_->state_ ==  MultiWordCompareAndSwap<T>::MCasState::PASS) {
      return this->cas_row_->new_value_;
    }

    return this->cas_row_->expected_value_;
  }

 private:
  // The Row in the MultiWordCompareAndSwap operation this MCH was placed for
  CasRow<T> *cas_row_;
  // The MultiWordCompareAndSwap which contains the cas_row_
  MultiWordCompareAndSwap<T> *mcas_op_;
};  // Helper



}  // namespace mcas
}  // namespace wf
}  // namespace algorithms
}  // namespace tervel

#endif  // TERVEL_MultiWordCompareAndSwap_MultiWordCompareAndSwap_HELPER_H_
