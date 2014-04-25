#ifndef TERVEL_MEMORY_HP_POOL_ELEMENT_H_
#define TERVEL_MEMORY_HP_POOL_ELEMENT_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include "tervel/memory/descriptor.h"
#include "tervel/memory/system.h"
#include "tervel/util.h"

namespace tervel {
namespace memory {

class Descriptor;

namespace hp {

/**
 *Unlike reference counting, this class does not need a struct to sepeate the
 * object from the header. As such Implementations can safely extend this class.
**/
class HPElement: public Descriptor {
 public:
    HPElement *next {nullptr}

#ifdef DEBUG_POOL

    std::atomic<bool> descriptor_in_use {false}

    // This stamp is checked when doing memory pool shenanigans to make sure
    // that a given descriptor actually belongs to a memory pool.
    int debug_pool_stamp {DEBUG_EXPECTED_STAMP}
#endif

  HPElement() { }
  ~HPElement() {
    descriptor_in_use.store(false);
  }
  /**
   * Helper method for getting the next pointer.
   */
  HPElement * next() { return header().next; }

  /**
   * Helper method for setting the next pointer.
   */
  void next(HPElement *next) { header().next = next; }

// -------
// Static Functions
// -------

/**
 * This method is used to increment the reference count of the passed descriptor
 * object. If after increming the reference count the object is still at the
 * address (indicated by *a == value), it will call advance_watch.
 * If that returns true then it will return true.
 * Otherwise it decrements the reference count and returns false
 *
 * @param the descriptor which needs rc protection, 
 * the address it was derferenced from, and the bitmarked value of it.
 */
static bool watch(HPElement *descr, std::atomic<void *> *a, void *value);

/**
 * This method is used to decrement the reference count of the passed descriptor
 * object.
 * Then it will call advance_unwatch and decrement any related objects necessary
 *
 * @param the descriptor which no longer needs rc protection.
 */
static void unwatch(HPElement *descr);

/**
 * This method is used to determine if the passed descriptor is under rc
 * rc protection.
 * Internally calls advance_iswatch.
 *
 * @param the descriptor to be checked for rc protection.
 */
static bool is_watched(HPElement *descr);



/**
 * This method is used to remove a descriptor object that is conflict with
 * another threads operation.
 * It first checks the recursive depth before proceding. 
 * Next it protects against the object being reused else where by acquiring
 * either HP or RC watch on the object.
 * Then once it is safe it will call the objects complete function
 * This function must gurantee that after its return the object has been removed
 * It returns the value.
 * 
 * @param a marked reference to the object and the address the object was
 * dereferenced from.
 */

static void * remove_hp_element(void *expected, std::atomic<void *> *address);

 private
  DISALLOW_COPY_AND_ASSIGN(HPElement);
};



}  // namespace hp
}  // namespace memory
}  // namespace tervel

#endif  // TERVEL_MEMORY_HP_POOL_ELEMENT_H_
