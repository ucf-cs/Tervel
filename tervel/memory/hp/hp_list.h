// REVIEW(carlos): guard variable name should be the full path to the file.
//   niether the ucf nor the thread directories exist anymore.
#ifndef UCF_THREAD_HP_DESCRIPTOR_POOL_H_
#define UCF_THREAD_HP_DESCRIPTOR_POOL_H_

#include <atomic>
#include <utility>

#include <assert.h>
#include <stdint.h>

// REVIEW(carlos): none of these include paths are correct. should be:
//   `tervel/memory/*'
#include "thread/descriptor.h"
#include "thread/info.h"
#include "thread/hp/pool_element.h"
#include "thread/hp/pool_manager.h"
#include "thread/system.h"

// REVIEW(carlos): should be `namespace tervel'
namespace ucf {
namespace memory {
namespace hp {
// REVIEW(carlos): please put a blank line between namespace block and beginning
//   of "real" code
class HPElement;
class ListManager;

// REVIEW(carlos): prefix HP on the List is redundant given the namespace name
//   of hp.
class HPList {
  // REVIEW(carlos): class comment should be above opening of class body
  // REVIEW(carlos): grammar: second sentence - linked *list*
  // REVIEW(carlos): remove extra blank line at bottom of comment
  /**
   * Defines a list of objects which are stored until they are safe to be freed.
   *
   * The list is represented as a linked lists of HP Elements
   *
   * Further, this object has a parent who is shared amongst other threads.
   * When it is to be destroyed, it sends its remaining elements to the
   * parent, relinquishing ownership of said elements. 
   *
   */
 public:
  friend HPElement;
  // REVIEW(carlos): if function body is empty, save a line with the
  //   spiky lemon: {} Save another line by moving the initializer list to the
  //   same line as ths constructor.
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

  // REVIEW(carlos): @params -> @param
  /**
   * Tries to free elements from the unsafe list.
   * @params dont_check If true, it ignores safty checks 
   */
  void try_to_free_HPElements(bool dont_check = false);


  // -------
  // MEMBERS
  // -------

  // REVIEW(carlos): grammar: lists's -> list's
  /**
   * This lists's manager.
   */
  ListManager *manager_;

  // REVIEW(carlos): excess vertical whitespace at bottom of comment
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
// REVIEW(carlos): should be `namespace tervel'

// REVIEW(carlos): include guard variable name is not right
#endif  // UCF_THREAD_HP_DESCRIPTOR_POOL_H_
