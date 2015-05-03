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

#ifndef TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_IMP_H_
#define TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_IMP_H_
namespace tervel {
namespace containers {
namespace lf {

template<typename T>
RingBuffer<T>::
RingBuffer(size_t capacity)
  : capacity_(capacity)
  , array_(new std::atomic<uintptr_t>[capacity]) {
  for (uint64_t i = 0; i < capacity_; i++) {
    array_[i].store(EmptyType(i));
  }
}

template<typename T>
bool RingBuffer<T>::
isFull() {
  return isFull(tail_.load(), head_.load());
}

template<typename T>
bool RingBuffer<T>::
isFull(int64_t tail, int64_t head) {
  int64_t temp = tail - head;
  return temp >= capacity_;
}


template<typename T>
bool RingBuffer<T>::
isEmpty() {
  return isEmpty(tail_.load(), head_.load());
}

template<typename T>
bool RingBuffer<T>::
isEmpty(int64_t tail, int64_t head) {
  int64_t temp = tail - head;
  return temp <= 0;
}

template<typename T>
void RingBuffer<T>::
atomic_delay_mark(int64_t pos) {
  array_[pos].fetch_or(delayMark_lsb);
}

template<typename T>
bool RingBuffer<T>::
readValue(int64_t pos, uintptr_t &val) {
  val = array_[pos].load();
  if (Helper::isHelperType(val)) {
    Helper * h = Helper::getHelperType(val);
    std::atomic<void *> *address;
    address = reinterpret_cast<std::atomic<void *> *>(&(array_[pos]));
    typedef tervel::util::memory::hp::HazardPointer::SlotID SlotID;
    SlotID pos = SlotID::SHORTUSE;
    bool res;
    res = tervel::util::memory::hp::HazardPointer::watch(pos, h, address, h);
    assert(!res);
    return false;
  } else {
    return true;
  }
}

template<typename T>
void RingBuffer<T>::
getInfo(uintptr_t val, int64_t &val_seqid,
    bool &val_isValueType, bool &val_isDelayedMarked) {
  val_isValueType = isValueType(val);
  val_isDelayedMarked = isDelayedMarked(val);
  if (val_isValueType) {
    val_seqid = getValueTypeSeqId(val);
  } else {
    val_seqid = getEmptyTypeSeqId(val);
  }
}

template<typename T>
T RingBuffer<T>::
getValueType(uintptr_t val) {
  val = val & (~clear_lsb);  // ~clear_lsb == 111...000
  T temp = reinterpret_cast<T>(val);
  return temp;
}

template<typename T>
bool RingBuffer<T>::
dequeue(T &value) {
  tervel::util::ProgressAssurance::check_for_announcement();
  util::ProgressAssurance::Limit progAssur;

  bool retry = true;
  while(retry) {
    if (isEmpty()) {
      return false;
    } else if(progAssur.isDelayed()) {
      break;
    }

    int64_t seqid = nextHead();
    uint64_t pos = getPos(seqid);
    uintptr_t val;
    uintptr_t new_value = EmptyType(nextSeqId(seqid));

   bool skip_delay_check = true;
    while (retry) {
      if (skip_delay_check) {
        // Removes a double increment on isDelayed
        skip_delay_check = false;
      } else if (progAssur.isDelayed()) {
        retry = false;
        break;
      }

      if (!readValue(pos, val)) {
        continue;
      }

      int64_t val_seqid;
      bool val_isValueType;
      bool val_isDelayedMarked;
      getInfo(val, val_seqid, val_isValueType, val_isDelayedMarked);

      if (val_seqid > seqid) {
        break;
      }
      if (val_isValueType) {
        if (val_seqid == seqid) {
          if (val_isDelayedMarked) {
            new_value = DelayMarkValue(new_value);
            assert(isDelayedMarked(new_value));
          }
          value = getValueType(val);

          uintptr_t sanity_check = val;
          if (!array_[pos].compare_exchange_strong(val, new_value)) {
            assert(!val_isDelayedMarked && "This value changed unexpectedly, it should only be changeable by this thread except for bit marking");
            assert(DelayMarkValue(sanity_check) == val && "This value changed unexpectedly, it should only be changeable by this thread except for bit marking");
            new_value =  DelayMarkValue(new_value);
            bool res = array_[pos].compare_exchange_strong(val, new_value);
            assert(res && " If this assert hits, then somehow another thread changed this value, when only this thread should be able to.");
          }
          return true;
        } else { // val_seqod < seqid
          if (backoff(pos, val)) {
            // value changed
            continue; // process the new value.
          }
          // Value has not changed so lets skip it.
          if (val_isDelayedMarked) {
            // Its marked and the seqid is less than ours so we
            // can skip it safely.
            break;
          } else {
            // we blindly mark it and re-examine the value;
            atomic_delay_mark(pos);

            continue;
          }
        }
      } else { // val_isEmptyType
        if (val_isDelayedMarked) {
          int64_t cur_head = getHead();
          int64_t temp_pos = getPos(cur_head);
          // We want to ensure that it has not been assigned.
          // So we move it up a head.
          cur_head += 2*capacity_ - temp_pos + pos;
          uintptr_t temp = EmptyType(cur_head);
          array_[pos].compare_exchange_strong(temp, new_value);
          continue;
        }
        if (!backoff(pos, val)) {
          // Value has not changed
          if (array_[pos].compare_exchange_strong(val, new_value)) {
            break;
          }
        }
        // Value has changed
        continue;
      }
    }  // inner loop
  } // outer loop.

  DequeueOp *op = new DequeueOp(this);
  tervel::util::ProgressAssurance::make_announcement(op);
  bool res = op->result(value);
  op->safe_delete();
  return res;
}


template<typename T>
bool RingBuffer<T>::
enqueue(T value) {
  tervel::util::ProgressAssurance::check_for_announcement();
  util::ProgressAssurance::Limit progAssur;

  bool retry = true;
  while(retry) {
    if (isFull()) {
      return false;
    } else if (progAssur.isDelayed()) {
      break;
    }

    int64_t seqid = nextTail();
    uint64_t pos = getPos(seqid);
    uintptr_t val;

    bool skip_delay_check = true;
    while (retry) {
      if (skip_delay_check) {
        // We skip the first iteration of this loop?
        // Reduces a double count
        skip_delay_check = false;
      } else if (progAssur.isDelayed()) {
        retry = false;
        break;
      }

      if (!readValue(pos, val)) {
        continue;
      }

      int64_t val_seqid;
      bool val_isValueType;
      bool val_isDelayedMarked;
      getInfo(val, val_seqid, val_isValueType, val_isDelayedMarked);

      if (val_seqid > seqid) {
        break;
      }


      if (val_isDelayedMarked) {
        // only a dequeue can update this value
        // lets backoff and see if it changes
        if (backoff(pos, val)) {
          // the value changed
          continue;
        } else {
          break; // get a new seqid
        }
      } else if (isEmptyType(val)) {
        int64_t other_seqid = getEmptyTypeSeqId(val);
        if (other_seqid < seqid) {
          if (backoff(pos, val)) {
            // value changed
            continue; // process the new value.
          }
        }
        // The current value is an EmptyType and its seqid is <= the assigned one.
        uintptr_t new_value = ValueType(value, seqid);
        if (array_[pos].compare_exchange_strong(val, new_value)) {
          return true;
        } else {
          // The position was updated and the latest value assigned to val.
          // So we need to reprocess it.
          continue;
        }

      } else {  // (isValueType(val)) {
        if (backoff(pos, val)) {
          // value changed
          continue; // process the new value.
        } else {
          // Value has not changed so lets skip it.
          break;
        }
      }
    }  // inner while(progAssur.notDelayed())
  }  // outer while(progAssur.notDelayed())

  EnqueueOp *op = new EnqueueOp(this, value);
  tervel::util::ProgressAssurance::make_announcement(op);
  bool res = op->result();
  op->safe_delete();
  return res;

}


template<typename T>
int64_t RingBuffer<T>::counterAction(std::atomic<int64_t> &counter, int64_t val) {
  int64_t seqid = counter.fetch_add(val);
  uint64_t temp = ~0x0;
  temp = temp >> num_lsb;

  assert( (seqid == ((seqid << num_lsb) >> num_lsb)) && "Seqid is too large the ring buffer should be recreated before this happens");
  return seqid;
}


template<typename T>
int64_t RingBuffer<T>::getHead() {
  return counterAction(head_, 0);
}

template<typename T>
int64_t RingBuffer<T>::casHead(int64_t &expected, int64_t new_val) {
  return head_.compare_exchange_strong(expected, new_val);
}

template<typename T>
int64_t RingBuffer<T>::nextHead() {
  return counterAction(head_, 1);
}


template<typename T>
int64_t RingBuffer<T>::getTail() {
  return counterAction(tail_, 0);
}

template<typename T>
int64_t RingBuffer<T>::casTail(int64_t &expected, int64_t new_val) {
  return tail_.compare_exchange_strong(expected, new_val);
}

template<typename T>
int64_t RingBuffer<T>::nextTail() {
  return counterAction(tail_, 1);
}




template<typename T>
uintptr_t RingBuffer<T>::EmptyType(int64_t seqid) {
  uintptr_t res = seqid;
  res = res << num_lsb; // 3LSB now 000
  res = res | emptytype_lsb; // 3LSB now 010
  return res;
}

template<typename T>
uintptr_t RingBuffer<T>::ValueType(T value, int64_t seqid) {
  value->func_seqid(seqid);
  uintptr_t res = reinterpret_cast<uintptr_t>(value);
  assert((res & clear_lsb) == 0 && " reserved bits are not 0?");
  return res;
}

template<typename T>
uintptr_t RingBuffer<T>::DelayMarkValue(uintptr_t val) {
  val = val | delayMark_lsb; // 3LSB now X1X
  return val;
}

template<typename T>
int64_t RingBuffer<T>::getEmptyTypeSeqId(uintptr_t val) {
  int64_t res = (val >> num_lsb);
  return res;
}
template<typename T>
int64_t RingBuffer<T>::getValueTypeSeqId(uintptr_t val) {
  T temp = getValueType(val);
  int64_t res = temp->func_seqid();
  return res;
}

template<typename T>
bool RingBuffer<T>::isEmptyType(uintptr_t p) {
  return (p & emptytype_lsb) == emptytype_lsb;
}

template<typename T>
bool RingBuffer<T>::isValueType(uintptr_t p) {
  return !isEmptyType(p);
}

template<typename T>
bool RingBuffer<T>::isDelayedMarked(uintptr_t p) {
  return (p & delayMark_lsb) == delayMark_lsb;
}

template<typename T>
intptr_t RingBuffer<T>::nextSeqId(int64_t seqid) {
  return seqid + capacity_;
}

template<typename T>
int64_t RingBuffer<T>::getPos(int64_t seqid) {
  int64_t temp = seqid % capacity_;
  assert(temp >= 0);
  assert(temp < capacity_);
  return temp;
}

template<typename T>
bool RingBuffer<T>::backoff(int64_t pos, uintptr_t val) {
  // TODO(steven): remove and replace with a call to tervels backoff
  std::this_thread::yield();
  uintptr_t nval = array_[pos].load();
  if (nval == val) {
    return false;
  } else {
    return true;
  }
}


template<typename T>
std::string RingBuffer<T>::debug_string(uintptr_t val) {
  int64_t val_seqid;

  bool val_isValueType;
  bool val_isDelayedMarked;
  getInfo(val, val_seqid, val_isValueType, val_isDelayedMarked);

  int64_t pos = getPos(val_seqid);
  std::string res = "{";
  res += "M: " + std::to_string(val_isDelayedMarked) + "\t";
  res += "V: " + std::to_string(val_isValueType) + "\t";
  res += "ID: " + std::to_string(val_seqid) + "\t";
  res += "Pos: " + std::to_string(pos) + "\t";
  res += "}";
  if (val_isValueType) {
    T temp = getValueType(val);
    res += "[" + temp->toString() +"]";
  }
  assert(!val_isDelayedMarked);
  return res;
};

template<typename T>
std::string RingBuffer<T>::debug_string() {
  std::string res = "";

  int64_t temp = head_.load();
  int64_t temp2 = tail_.load();
  res += "Head: " + std::to_string(temp) + "\t"
      + " Pos: " + std::to_string(getPos(temp)) + " \n"
      + "Tail: " + std::to_string(temp2) + "\t"
      + "Pos: " + std::to_string(getPos(temp2)) + " \n"
      + "capacity_: " + std::to_string(capacity_) + "\n";
  if (isFull()) {
    res += "isFull: True\n";
  } else {
    res += "isFull: False\n";
  }
  if (isEmpty()) {
    res += "isEmpty: True\n";
  } else {
    res += "isEmpty: False\n";
  }
  for (int  i = 0; i < capacity_; i++) {
    res += "[" + std::to_string(i) + "] ";
    uintptr_t val = array_[i].load();
    res += debug_string(val);
    res += "\n";
  }

  if (isEmpty()) {
    assert(!isFull());
  }
  if (isFull()) {
    assert(!isEmpty());
  }
  return res;
};

}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_IMP_H_
