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
#ifndef __TERVEL_CONTAINERS_WF_VECTOR_PUSHWRA_OP_H
#define __TERVEL_CONTAINERS_WF_VECTOR_PUSHWRA_OP_H


#include <tervel/util/info.h>
#include <tervel/util/descriptor.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/hp/hazard_pointer.h>
#include <tervel/util/memory/rc/descriptor_util.h>


#include <tervel/containers/wf/vector/vector.hpp>

namespace tervel {
namespace containers {
namespace wf {
namespace vector {

template<typename T>
class PushWRAOpHelper;


template<typename T>
class PushWRAOp: public tervel::util::OpRecord {
public:
  PushWRAOp(Vector<T> *vec, T val)
   : vec_(vec)
   , new_val_(val) {}

  ~PushWRAOp() {
    PushWRAOpHelper<T> * temp = helper_.load();
    assert(temp != nullptr);
    util::memory::rc::free_descriptor(temp, true);
  }

  void help_complete() {
    tervel::tl_control_word = &helper_;

    PushWRAOpHelper<T> *helper = tervel::util::memory::rc::get_descriptor<
    		PushWRAOpHelper<T> >(this);
    T help_t = reinterpret_cast<T>(util::memory::rc::mark_first(helper));

    size_t placed_pos = vec_->size();
    std::atomic<T> *spot = vec_->internal_array.get_spot(placed_pos);
    T current = spot->load();

    while (helper_.load() == nullptr) {
      if (current == Vector<T>::c_not_value_) {

	helper->set_idx(placed_pos);
	if (!spot->compare_exchange_strong(current, help_t)) {
	  continue;
	}
	helper->complete(reinterpret_cast<void *>(help_t),
		  reinterpret_cast< std::atomic<void *> *>(spot));

	assert(helper_.load() == helper);
	return;
      } else if (vec_->internal_array.is_descriptor(current, spot)) {
	continue;
      } else {  // its a valid value
	placed_pos++;
	spot = vec_->internal_array.get_spot(placed_pos);
	current = spot->load();
      }
    }
    util::memory::rc::free_descriptor(helper, true);


  };

  uint64_t result() {
    PushOpHelper<T> *temp = helper_.load();
    assert(temp != nullptr);
    return temp->idx();
  }

private:
  friend class PushWRAOpHelper<T>;
  Vector<T> *vec_;
  T new_val_;
  std::atomic<PushWRAOpHelper<T> *> helper_ {nullptr};

};  // class PushWRAOp


template<typename T>
class PushWRAOpHelper: public tervel::util::Descriptor {
public:
 explicit PushWRAOpHelper(PushWRAOp<T> *op)
   : op_(op)
   {}

 void set_idx(uint64_t i) {
   idx_ = i;
 };

 uint64_t idx() {
   return idx_;
 };

 bool result() {
   if (op_->helper_.load() == nullptr) {
     return associate();
   } else if (op_->helper_.load() == this) {
     return true;
   } else {
     assert(op_->helper_.load() != nullptr);
     return false;
   }
 };

 bool associate() {
   PushWRAOpHelper *temp_null = nullptr;
   bool res = op_->helper_.compare_exchange_strong(temp_null, this);
   if (res || temp_null == this) {
     assert(op_->helper_.load() == this);
     return true;
   } else {
     assert(false);
     return false;
   }
 };

 using util::Descriptor::complete;
 void * complete(void *value, std::atomic<void *> *address) {
   std::atomic<T> * spot = reinterpret_cast<std::atomic<T> *>(address);
   bool op_res = associate();
   T new_current = reinterpret_cast<T>(util::memory::rc::mark_first(this));
   assert(reinterpret_cast<T>(value) == new_current);
//   if (op_res) {
     if (spot->compare_exchange_strong(new_current, op_->new_val_)) {
       new_current = op_->new_val_;
     }
     //SEE: associate()
/*   } else {
     if (spot->compare_exchange_strong(new_current,
           reinterpret_cast<T>(Vector<T>::c_not_value_))) {
       new_current = reinterpret_cast<T>(Vector<T>::c_not_value_);
     }
   }*/
   return reinterpret_cast<void *>(new_current);
 }  // complete

 using util::Descriptor::get_logical_value;
 void * get_logical_value() {
     return reinterpret_cast<void *>(op_->new_val_);
 }  // get_logical_value

 /**
  * This function is called after this objects rc count was incremented.
  * It acquires a  HP watch on the PushOp op,
  *
  * @param address the address this PushOpHelper was read from
  * @param value the bitmarked value of this WriteHelper
  * @return returns whether or not the watch was successful.
  */
 using util::Descriptor::on_watch;
 bool on_watch(std::atomic<void *> *address, void * value) {
   typedef util::memory::hp::HazardPointer::SlotID t_SlotID;
   bool success = util::memory::hp::HazardPointer::watch(
         t_SlotID::SHORTUSE, op_, address, value);

   if (success) {
     complete(value, address);
   }
   return false;
 };

private:
  friend class PushWRAOp<T>;
  PushWRAOp<T> *op_;
  uint64_t idx_;
//  std::atomic<uint64_t> success_ {0};

};  // class PushWRAOpHelper

}  // namespace vector
}  // namespace wf
}  // namespace containers
}  // namespace tervel
#endif  // __TERVEL_CONTAINERS_WF_VECTOR_PUSHWRA_OP_H
