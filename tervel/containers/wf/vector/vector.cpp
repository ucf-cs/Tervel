#ifndef __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_CPP_
#define __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_CPP_


#include "tervel/containers/wf/write_op.h"
#include "tervel/containers/wf/read_op.h"
#include "tervel/containers/wf/vector_array.h"

template<class T>
size_t Vector::push_back_only(T value) {
  if (!Value::isValid(value)) {
    assert(false);
    return -1;
  }

  size_t placed_pos = size(1);
  ArrayElement *spot = internal_array.get_spot(placed_pos);

  spot->store(value, std::memory_order_relaxed);
  return placed_pos;
}  // push_back_only

template<class T>
size_t Vector::push_back_w_ra(T value) {
  tervel::util::ProgressAssurance::check_for_announcement();

  if (!Value::isValid(value)) {
    assert(false);
    return -1;
  }

  size_t placed_pos = size();
  while (true) {  // TODO(steven): TO BOUND
    ArrayElement *spot = internal_array.get_spot(placed_pos);

    T expected=  spot->load();

    if (internal_array.is_descriptor(expected, spot)) {
      continue;
    } else if ( (expected ==  c_not_value_) &&
         (spot->compare_exchange_weak(expected, value) ) {
      size(1);
      return placed_pos;
    }
    placed_pos++;
  }  // while not complete
}  // push_back_w_ra

template<class T>
bool Vector::pop_back_only(T &value) {
  size_t poped_pos = size(-1);

  if (poped_pos <= 0) {
    size(1)
    return false;
  } else {
    ArrayElement *spot = internal_array.get_spot(poped_pos);
    value = spot->load(std::memory_order_relaxed);
    spot->store(c_not_value_, std::memory_order_relaxed);

    return true;
  }
}  // pop_back_only

template<class T>
bool Vector::pop_back_w_ra(T &value) {
  tervel::util::ProgressAssurance::check_for_announcement();

  size_t poped_pos = size();

  while (true) {  // TODO(steven): TO BOUND
    if (poped_pos <= 0) {
      return false;
    }

    ArrayElement *spot = internal_array.get_spot(poped_pos - 1);
    T current = spot->load();

    if (internal_array.is_descriptor(expected, spot)) {
      continue;
    } else if (spot->compare_exchange_weak(current, c_not_value_)) {
      size(-1);
      value = current;
      return true;
    }
    poped_pos--;
  }  // while not complete
}  // pop_back_w_ra

/*
template<class T>
size_t Vector::push_back(T value) {
  tervel::util::ProgressAssurance::check_for_announcement();

  if(!Value::isValid(value)){
    assert(false);
    return -1;
  }

  PushOp<T>* op = new PushOp<T>(this, value);
  size_t pos = op->begin(this);
  op->safe_delete();

  size(1);
  return pos;
}

template<class T>
bool Vector::pop_back(const T &value) {
  tervel::util::ProgressAssurance::check_for_announcement();

  PopOp<T> *op = new PopOp<T>(this);
  bool op_succ = op->begin(value);
  op->safe_delete();

  if (op_succ) {
    size(-1);
  }
  return op_succ;
} */

template<class T>
bool Vector::at(int idx, const T &value) {
  tervel::util::ProgressAssurance::check_for_announcement();

  std::atomic<T> control_address(nullptr);
  tl_control_word = &control_address;

  if (idx < capacity()) {
    ArrayElement *spot = internal_array.get_spot(idx, false);

    size_t fcount = 0;
    while (fcount++ < util::ProgressAssurance::MAX_FAILURES) {
      T cvalue = spot->load(std::memory_order_relaxed);

      if (internal_array.is_descriptor(expected, spot)) {
        continue;
      } else if (cvalue == c_not_value_) {
        return false;
      } else {
        assert(Value::isValid(cvalue));
        value = cvalue;
        return true;
      }
    }  // while fail threshold has not been reached

    ReadOp<T> *op = new ReadOp<T>(this, idx);
    tl_control_word=&(op->value);

    util::ProgressAssurance::make_announcement(
          reinterpret_cast<tervel::util::OpRecord *>(op));

    bool op_succ = op->result(value);
    op->safe_delete();

    return op_succ;
  }  // if idx < capacity()

  return false;
};

template<class T>
bool Vector::cas(int idx, const T &expected, const T val) {
  tervel::util::ProgressAssurance::check_for_announcement();

  std::atomic<T> control_address(nullptr);
  tl_control_word = &control_address;

  if (idx < capacity()) {
    ArrayElement *spot = internal_array.get_spot(idx, false);

    size_t fcount = 0;
    while (fcount++ < util::ProgressAssurance::MAX_FAILURES) {
      T cvalue = spot->load(std::memory_order_relaxed);

      if (internal_array.is_descriptor(expected, spot)) {
        continue;
      } else if (cvalue == expected) {
        T temp = expected;
        bool suc = spot->compare_exchange_strong(temp, val);
        if (suc) {
          return suc;
        } else if (internal_array.is_descriptor(temp, spot)) {
          continue;
        } else {
          expected = temp;
          return false;
        }
      } else {
        return expected = cvalue;
      }
    }  // while fail threshold has not been reached

    WriteOp<T> *op = new WriteOp<T>(this, idx, expected, val);
    tl_control_word=&(op->value);

    util::ProgressAssurance::make_announcement(
          reinterpret_cast<tervel::util::OpRecord *>(op));

    bool op_succ = op->result(expected);
    op->safe_delete();

    return op_succ;
  }  // if idx < capacity()

  expected = c_not_value_;
  return false;
};

/*
template<class T>
bool Vector::insertAt(int idx, T value){
  tervel::util::ProgressAssurance::check_for_announcement();

  InsertAt<T>* op = new InsertAt<T>(idx, value);
  tl_control_word = &(op->state);

  bool res=op->begin(this);

  if (res) {
    op->cleanup(this, idx);
    size(1);
  }

  op->safe_delete();
  return res;
};

template<class T>
bool Vector::eraseAt(int idx, T &value){
  tervel::util::ProgressAssurance::check_for_announcement();

  EraseAt<T>* op = new EraseAt<T>(idx);
  tl_control_word = &(op->state);

  bool res=op->begin(this, value);

  if (res) {
    op->cleanup(this, idx);
    size(-1);
  }

  op->safe_delete();
  return res;
}; */


#endif  // __TERVEL_CONTAINERS_WF_VECTOR_VECTOR_CPP_