#ifndef UCF_THREAD_HP_DESCRIPTOR_POOL_H_
#define UCF_THREAD_HP_DESCRIPTOR_POOL_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>

#include "thread/descriptor.h"
#include "thread/info.h"
#include "thread/hp/pool_element.h"
#include "thread/hp/pool_manager.h"
#include "thread/system.h"

namespace ucf {
namespace thread {
namespace hp {
class HPElement;
class ListManager;


/**
 * Defines a pool of objects which are stored until they are safe to be freed.
 *
 * The list is represented as a linked lists of HP Elements
 *
 * Further, this object has a parent who is shared amongst other threads.
 * When it is to be destroyed, it sends its remaining elements to the
 * parent, relinquishing ownership of said elements. 
 *
 */
class HPList {
 public:
  friend HPElement;
  explicit HPList(ListManager *manager)
      : manager_(manager) {
  }
  ~HPList() { this->send_to_manager(); }


 private:
  // -------------------------
  // FOR DEALING WITH MANAGERS
  // -------------------------

  /**
   * Sends all elements managed by this list to the parent.
   */
  void send_to_manager();


  // --------------------------------
  // DEALS WITH UNSAFE LIST
  // --------------------------------
  /**
   * This function adds an HPElement to the unsafe list.
   * @param elem The element to add
   */
  void add_to_unsafe(HPElement* elem);

  /**
   * Tries to free elements from the unsafe list.
   * @params dont_check If true, it ignores safty checks 
   */
  void try_to_free_HPElements(bool dont_check = false);


  // -------
  // MEMBERS
  // -------

  /**
   * This lists's manager.
   */
  ListManager *manager_;

  /**
   * A linked list of list elements. 
   * HPElements are freed when they are no longer referenced by other threads.
   * 
   */
  HPElement *unsafe_list_ {nullptr}
};

}  // namespace hp
}  // namespace thread
}  // namespace ucf

#endif  // UCF_THREAD_HP_DESCRIPTOR_POOL_H_
