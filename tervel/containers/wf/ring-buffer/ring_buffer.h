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


/**
 * TODO(steven):
 *
 *   Add static type checking of template Type T
 *
 *   Annotate code a bit more.
 *
 */
#ifndef TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_H_
#define TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_H_

#include <atomic>
#include <assert.h>
#include <cstddef>
#include <memory>
#include <thread>
#include <string>

#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/progress_assurance.h>
#include <tervel/util/memory/hp/hazard_pointer.h>

namespace tervel {
namespace containers {
namespace wf {

/**
 * @brief This is a non-blocking FIFO ring buffer design
 * that was made wait-free by applying a progress assurance framework to it.
 *
 * If isDelayed never returns true then it will not use additional memory.
 * However, if it does, then it will allocate some objects which are protected
 * by hazard pointer protection.
 *
 * @details This ring buffer is implemented on a statically sized array.
 * It can only store pointer sized objects that extend the Value class.
 * Further it reserves the 3 LSB for type identification, which makes it
 * compatible only with 64 bit systems
 *
 * It supports enqueue, dequeue, isFull, and isEmpty operations
 *
 * @tparam T The type of information stored, must be a pointer and the class
 * must extend RingBuffer::Value.
 */
template<typename T>
class RingBuffer {
  static const uintptr_t num_lsb = 3;
  static const uintptr_t delayMark_lsb = 0x1;
  static const uintptr_t emptytype_lsb = 0x2;
  static const uintptr_t oprec_lsb = 0x4;
  static const uintptr_t clear_lsb = 7;

  static_assert(sizeof(T) == sizeof(uintptr_t) &&
    sizeof(uintptr_t) == sizeof(uint64_t), " Pointers muse be 64 bits");

 public:
  /**
   * @brief RingBuffer value class, values stored in the class must extend it.
   * @details
   * This class is necessary to provide the FIFO property.
   * It adds a sequence identifier to each value stored.
   * Using this identifier, we are apply generate FIFO valid sequential history
   * from a concurrent history.
   */
  class Value {
   public:
    /**
     * @brief Empty constructor
     * @details Empty Constructor
     */
    Value() {};
    friend RingBuffer;
   private:
    /**
     * @brief Returns the items seqid
     * @details Returns the items seqid
     * @return Returns the items seqid
     */
    int64_t func_seqid() {
      assert(seqid_ != -1);
      return seqid_;
    }

    /**
     * @brief Sets the items seqid
     * @details Sets the items seqid
     * @return Sets the items seqid
     */
    void func_seqid(int64_t s) {
      seqid_ = s;
    }

    /**
     * @brief Conditionally updates the seqid
     * @details This function is used when multiple threads maybe enqueueing the
     * same value
     *
     * @param e expected seqid (address of the oprec * -1).
     * @param n new seqid
     */
    void atomic_change_seqid(int64_t e, int64_t n) {
      std::atomic<int64_t> *a;
      a = reinterpret_cast<std::atomic<int64_t> *>(&seqid_);
      bool res = a->compare_exchange_strong(e, n);
      assert( (res || e ==n) && " Seqid changed to an unexpected value in the"
        " progress assurance scheme");
    }
    int64_t seqid_{-1};
  };

  /**
   * @brief Ring Buffer constructor
   * @details This constructs and initializes the ring buffer object
   *
   * @param capacity the length of the internal array to allocate.
   */
  RingBuffer(size_t capacity);

  /**
   * @brief Returns whether or not the ring buffer is full.
   * @details Returns whether or not the ring buffer is full.
   * @return Returns whether or not the ring buffer is full.
   */
  bool isFull();

  /**
   * @brief Returns whether or not the ring buffer is full.
   * @details Returns whether or not the ring buffer is full.
   *
   * @param tail [description]
   * @param head [description]
   *
   * @return Returns whether or not the ring buffer is full.
   */
  bool isFull(int64_t tail, int64_t head);


  /**
   * @brief Returns whether or not the ring buffer is empty.
   * @details Returns whether or not the ring buffer is empty.
   * @return Returns whether or not the ring buffer is empty.
   */
  bool isEmpty();

  /**
   * @brief Returns whether or not the ring buffer is empty.
   * @details Returns whether or not the ring buffer is empty.
   * @param tail
   * @param head
   *
   * @return Returns whether or not the ring buffer is empty.
   */
  bool isEmpty(int64_t tail, int64_t head);

  /**
   * @brief Enqueues the passed value into the buffer
   * @details This function attempts to enqueue the passed value.
   * It returns false in the event the ring buffer is full.
   * Internally it assigned a sequence number to the value.
   *
   * @param value The value to enqueue.
   * @return whether or not the value was enqueued.
   */
  bool enqueue(T value);

  /**
   * @brief Dequeues a value from the buffer
   * @details This function attempts to dequeue a value.
   * It returns false in the event the ring buffer is empty.
   *
   * @param value A variable to store the dequeued value.
   * @return whether or not a value was dequeued.
   */
  bool dequeue(T &value);

  /**
   * @brief This function returns a string debugging information
   * @details This information includes
   * Value at each potion, head, tail, capacity, and someother stuff.
   * @return debugging info string
   */
  std::string debug_string();

 private:

  /**
   * @brief This function attempts to load a value from the buffer
   * @details This function encapsulates logic related to achieving hazard
   * pointer protection on objects read from the buffer. Currently it does
   * not protected type T objects, only Helper objects used in the progress
   * assurance framework.
   * If a Helper object is read it will return false, otherwise it will return
   * a successful read.
   *
   * @param pos the position to load from
   * @param val the variable to store the loaded value.
   *
   * @return whether or not a load was successful
   */
  bool readValue(int64_t pos, uintptr_t &val);

  /**
   * @brief Creates a uintptr_t that represents an EmptyType
   * @details the uintptr_t is composed by
   * seqid << num_lsb | emptytype_lsb
   *
   * @param seqid the seqid for this EmptyType
   * @return Returns uintptr_t that represents an EmptyType
   */
  static inline uintptr_t EmptyType(int64_t seqid);

  /**
   * @brief Returns the seqid encoded in the passed value
   * @details Returns the seqid encoded in the passed value
   *
   * seqid = val >> num_lsb
   *
   * @param val a unitptr_t loaded from the buffer
   * @return its encoded seqid
   */
  static inline int64_t getEmptyTypeSeqId(uintptr_t val);

  /**
   * @brief Creates a uintptr_t that represents an ValueType
   * @details the uintptr_t is composed by casting value to a uintptr_t.
   *
   * Internally it calls value->func_seqid(seqid) to set the seqid.
   *
   * @param value a pointer type to enqueue
   * @param seqid the sequence id assigned to this value
   * @return Returns uintptr_t that represents an ValueType
   */
  static inline uintptr_t ValueType(T value, int64_t seqid);

  /**
   * @brief Returns the value type from a uintptr
   * @details clears any delayed marks then casts it to type T.
   *
   * @param val The value to cast
   * @return val cast to type T
   */
  static inline T getValueType(uintptr_t val);

  /**
   * @brief Returns the seqid of the passed value
   * @details Returns the seqid of the passed value, first it casts val to type
   * T by calling getValyeType then it calls val->func_seqid();
   *
   * @param val a value read from the ring buffer that has been determined to
   * be a ValueType
   * @return The values seqid
   */
  static inline int64_t getValueTypeSeqId(uintptr_t val);

  /**
   * @brief Takes a uintptr_t and places a bitmark on the delayMark_lsb
   * @details Takes a uintptr_t and places a bitmark on the delayMark_lsb
   *
   * @param val The value to mark
   * @return A marked value
   */
  static inline uintptr_t DelayMarkValue(uintptr_t val);

  /**
   * @brief This function is used to get information from a value read from the
   * ring buffer
   * @details This function is used to get information from a value read from the
   * ring buffer. This information is then assigned to the arguments.
   *
   * @param val a value read from a position on the ring buffer
   * @param val_seqid The seqid associated with val
   * @param val_isValueType Whether or not val is a ValueType
   * @param val_isMarked Whether or not val has a delay mark.
   */
  void getInfo(uintptr_t val, int64_t &val_seqid,
    bool &val_isValueType, bool &val_isMarked);

  /**
   * @brief returns whether or not p represents an EmptyType
   * @details returns whether or not p represents an EmptyType by examining
   * the emptytype_lsb. If it is 1 then it is an EmptyType type.
   *
   * @param p the value to examine
   * @return whether or not it is an EmptyType
   */
  static inline bool isEmptyType(uintptr_t p);

  /**
   * @brief returns whether or not p represents an ValueType
   * @details returns whether or not p represents an ValueType by examining
   * the emptytype_lsb. If it is 0 then it is an ValueType type.
   *
   * @param p the value to examine
   * @return whether or not it is an ValueType
   */
  static inline bool isValueType(uintptr_t p);

  /**
   * @brief returns whether or not p has a delay mark
   * @details returns whether or not p has a delay mark by examining
   * the delayMark_lsb. If it is 1 then it is delayed
   *
   * @param p the value to examine
   * @return whether or not it is has a delay mark.
   */
  static inline bool isDelayedMarked(uintptr_t p);

  /**
   * @brief performs a fetch-and-add on the head counter
   * @details atomically increments the head
   * @return returns the pre-incremented value of the head counter.
   */
  int64_t nextHead();

  /**
   * @brief performs an atomic load on the head counter
   * @details performs an atomic load on the head counter
   * @return returns the value of the head counter.
   */
  int64_t getHead();

  int64_t casHead(int64_t &expected, int64_t new_val);

  /**
   * @brief performs a fetch-and-add on the tail counter
   * @details atomically increments the tail
   * @return returns the pre-incremented value of the tail counter.
   */
  int64_t nextTail();

  /**
   * @brief performs an atomic load on the tail counter
   * @details performs an atomic load on the tail counter
   * @return returns the value of the tail counter.
   */
  int64_t getTail();

  int64_t casTail(int64_t &expected, int64_t new_val);

  /**
   * @brief utility function for incrementing counter
   * @details contains internal checks on the returned value
   *
   * @param counter counter to increment
   * @param val value by which to increment the counter
   *
   * @return returns the pre-incremented value of the tail counter.
   */
  static inline int64_t counterAction(std::atomic<int64_t> &counter, int64_t val);

  /**
   * @brief Returns the next seqid
   * @details Returns the next seqid, which is seqid+capacity_
   *
   * @param seqid The current seqid
   * @return the next seqid
   */
  int64_t nextSeqId(int64_t seqid);

  /**
   * @brief Returns the position a seqid belongs at
   * @details Returns the position a seqid belongs at, determined by
   * seqid % capacity_
   *
   * @param seqid the seqid
   * @return the position the seqid belongs at
   */
  int64_t getPos(int64_t seqid);

  /**
   * @brief A backoff routine in the event of thread delay
   * @details This function is called in the event the value at position on the
   * ringbuffer is lagging behind as a result of a delayed thread.
   *
   * It calls tervel::util::backoff() and upon its return checks whether or
   * the value at address has changed.
   * If it has, it returns true, Else it returns false.
   * If the value has changed the new value is assigned to val
   *
   * @param pos The pos val was read from
   * @param val The last read value from address.
   *
   * @return whether or not the val changed.
   */
  bool backoff(int64_t pos, uintptr_t val);

  /**
   * @brief This function places a bitmark on the value held at address
   * @details This function places a bitmark on the value held at address by
   * calling address->fetch_or(delayMark_lsb) and then it loads the new current value
   * and assigns it to val.
   *
   * @param pos The position to perform the blind bitmark.
   */
  void atomic_delay_mark(int64_t pos);

  /**
   * @brief This prints out debugging information.
   * @details This prints out debugging information.
   *
   * @param val a value loaded from a position on the buffer
   * @return a string representation the contents of val.
   */
  std::string debug_string(uintptr_t val);


  class BufferOp;
  class EnqueueOp;
  class DequeueOp;
  class Helper;

  const int64_t capacity_;
  std::atomic<int64_t> head_ {0};
  std::atomic<int64_t> tail_ {0};
  std::unique_ptr<std::atomic<uintptr_t>[]> array_;

};  // class RingBuffer<Value>



}  // namespace wf
}  // namespace containers
}  // namespace tervel

#include <tervel/containers/wf/ring-buffer/ring_buffer_op.h>
#include <tervel/containers/wf/ring-buffer/helper.h>
#include <tervel/containers/wf/ring-buffer/enqueue_op.h>
#include <tervel/containers/wf/ring-buffer/dequeue_op.h>

#include <tervel/containers/wf/ring-buffer/helper_imp.h>
#include <tervel/containers/wf/ring-buffer/enqueue_op_imp.h>
#include <tervel/containers/wf/ring-buffer/dequeue_op_imp.h>

#include <tervel/containers/wf/ring-buffer/ring_buffer_imp.h>


#endif  // TERVEL_CONTAINERS_WF_RINGBUFFER_RINGBUFFER_H_