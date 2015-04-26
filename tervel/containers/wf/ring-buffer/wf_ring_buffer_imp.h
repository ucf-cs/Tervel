
namespace tervel {
namespace containers {
namespace wf {

template<typename T>
RingBuffer<T>::RingBuffer(size_t capacity)
  : capacity_(capacity)
  , array_(new std::atomic<uintptr_t>[capacity]) {
  for (uint64_t i = 0; i < capacity_; i++) {
    array_[i].store(EmptyNode(i));
  }
}

template<typename T>
bool RingBuffer<T>::isFull() {
  return tail.load() - head.load() > capacity_;
}

template<typename T>
bool RingBuffer<T>::isEmpty() {
  return tail.load() - head.load() <= 0;
}

template<typename T>
bool RingBuffer<T>::dequeue(T value) {
  while(true) {
    if (isEmpty()) {
      return false;
    }

    uint64_t seqid = nextHead();
    uint64_t pos = getPos(seqid);
    uintptr_t val = array_[pos].load();
    uint64_t new_value = EmptyNode(nextSeqId(seqid));

    while (true) {
      uint64_t val_seqid;
      bool val_isDataNode;
      bool val_isMarked;
      getInfo(&val_seqid, &val_isDataNode, &val_isMarked);

      if (val_seqid > seqid) {
        break;
      }
      if (val_isDataNode) {
        if (val_seqid == seqid) {
          if (val_isMarked) {
            new_value =  MarkNode(new_value);
          }
          value = getValue(val);

          uint64_t sanity_check = val);
          if (!array_[pos].compare_exchange_strong(val, new_value)) {
            assert(!val_isMarked && "This value changed unexpectedly, it should only be changeable by this thread except for bit marking");
            assert(MarkNode(sanity_check) == val && "This value changed unexpectedly, it should only be changeable by this thread except for bit marking");
            new_value =  MarkNode(new_value);
            bool res = array_[pos].compare_exchange_strong(val, new_value)
            assert(res && " If this assert hits, then somehow another thread changed this value, when only this thread should be able to.");
          }
          return true;
        } else { // val_seqod < seqid
          if (backoff(&array_[pos], &val)) {
            // value changed
            continue; // process the new value.
          }
          // Value has not changed so lets skip it.
          if (val_isMarked) {
            // Its marked and the seqid is less than ours so we
            // can skip it safely.
            break;
          } else {
            // we blindly mark it and re-examine the value;
            delay_mark(&array_[pos]);
            continue;
          }
        }
      } else { // val_isEmptyNode
        if (val_isMarked || !backoff(&array_[pos], &val)) {
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
}


template<typename T>
bool RingBuffer<T>::enqueue(T value) {
  while(true) {
    if (isFull()) {
      return false;
    }

    uint64_t seqid = nextTail();
    uint64_t pos = getPos(seqid);
    uintptr_t val = array_[pos].load();

    while (true) {
      if (isDelayedMarked(val)) {
        // only a dequeue can update this value
        // lets backoff and see if it changes
        backoff();
        uintptr_t val2 = array_[pos].load();
        if (val != val2) {
          // The slow thread made its change, need to reprocess the current value.
          val = val2;
          continue;
        } else {
          break; // get a new seqid
        }
      } else if (isEmptyNode(val)) {
        uintptr_t other_seqid = getEmptyNodeSeqId(val);
        if (other_seqid > seqid) {
          // our seqid has been skipped, so we need a new one
          break;
        } else if (other_seqid < seqid) {
          // give a delayed thread a chance to catch up
          backoff();
          uintptr_t val2 = array_[pos].load();
          if (val != val2) {
            // The slow thread made its change, need to reprocess the current value.
            val = val2;
            continue;
          }
        }
        // The current value is an empty node and its seqid is <= the assigned one.
        uintptr_t new_value = DataNode(value, seqid);
        if (array_[pos].compare_exchange_strong(val, new_value)) {
          return true;
        } else {
          // The position was updated and the latest value assigned to val.
          // So we need to reprocess it.
          continue;
        }

      } else {  // (isDataNode(val)) {
        uintptr_t other_seqid = getDataNodeSeqId(val);
        if (other_seqid >= seqid) {
          // our seqid has been skipped, so we need a new one
          break;
        } else {
          backoff();
          uintptr_t val2 = array_[pos].load();
          if (val == val2) {
            // Another thread is being slow and we should not wait any longer
            break;
          } else {
            // Value changed lets re-read
            val = val2;
          }
        }
      }
    }  // inner while(true)
  }  // outer while(true)
}

template<typename T>
uint64_t RingBuffer<T>::nextHead() {
  uint64_t seqid = head.fetch_add(1);
  assert( (seqid & (~0x0 >> reserved_lsb)) == 0 && "Seqid is too large the ring buffer should be recreated before this happens");
  return seqid;
}

template<typename T>
uint64_t RingBuffer<T>::nextTail() {
  uint64_t seqid = tail.fetch_add(1);
  assert( (seqid & (~0x0 >> reserved_lsb)) == 0 && "Seqid is too large the ring buffer should be recreated before this happens");
  return seqid;
}

template<typename T>
uintptr_t RingBuffer<T>::EmptyNode(uint64_t seqid) {
  uintptr_t res = seqid;
  res = res << reserved_lsb; // 3LSB now 000
  res = res | emptynode_lsb; // 3LSB now 010
  return res;
}

template<typename T>
uintptr_t RingBuffer<T>::DataNode(T value, uint64_t seqid) {  // TODO(Steven): switch to pointer type
  uintptr_t res = seqid;
  res = res << reserved_lsb; // 3LSB now 000
  return res;
}

template<typename T>
uintptr_t RingBuffer<T>::MarkNode(uint64_t node) {
  node = node | mark_lsb; // 3LSB now X1X
  return node;
}

template<typename T>
uintptr_t RingBuffer<T>::getEmptyNodeSeqId(uint64_t emptyNode) {
  uintptr_t res = (emptyNode >> reserved_lsb);
  return res;
}
template<typename T>
uintptr_t RingBuffer<T>::getDataNodeSeqId(uint64_t DataNode) {
  // TODO(Steven): switch to pointer type
  uintptr_t res = (DataNode >> reserved_lsb);
  return res;
}

template<typename T>
uintptr_t RingBuffer<T>::isEmptyNode(uint64_t p) {
  return (p & emptynode_lsb) == emptynode_lsb;
}

template<typename T>
uintptr_t RingBuffer<T>::isDataNode(uint64_t p) {
  return !isEmptyNode(p);
}

template<typename T>
uintptr_t RingBuffer<T>::isDelayedMarked(uint64_t p) {
  return (p & mark_lsb) == mark_lsb;
}

template<typename T>
uintptr_t RingBuffer<T>::nextSeqId(uint64_t seqid) {
  return seqid + capacity_;
}

template<typename T>
uintptr_t RingBuffer<T>::getPos(uint64_t seqid) {
  return seqid % capacity_;
}

template<typename T>
void RingBuffer<T>::backoff(std::atomic<uintptr_t> *address, uintptr_t *val) {
  uintptr_t nval = address->load();
  if (nval == *val);
    return false;
  else {
    *val = nval;
    return true;
  }
}


}  // namespace wf
}  // namespace containers
}  // namespace tervel