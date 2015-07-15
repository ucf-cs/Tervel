#ifndef __TERVEL_CONTAINERS_LF_LINKED_LIST_QUEUE_LINKED_LIST
#define __TERVEL_CONTAINERS_LF_LINKED_LIST_QUEUE_LINKED_LIST


#include <tervel/util/info.h>
#include <tervel/util/util.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hazard_pointer.h>

namespace tervel {
namespace containers {
namespace lf {

template<typename T>
class LinkedList {
 public:

  class Node : public tervel::util::memory::hp::Element {
   public:
    static constexpr Node * EndMark = reinterpret_cast<Node *>(0x1L);


    Node(T &v, Node *next = nullptr)
     : v_ (v)
     , next_(nullptr) {};

    T& value() { return v_; }
    Node * next() { return next_.load(); }

    /**
     * @brief returns whether or not the next_ member variable was changed
     * @details using a compare_and_exchange operation on the next_ member,
     * try to replace nullptr with 'other'. Return whether or not the operation
     * was successful
     *
     * @param other the value to have next_ set to
     * @return whether or not it was success
     */
    bool set_next(Node * const other);

   private:
    T v_;
    std::atomic<Node *> next_ {nullptr};
  };

  class Accessor {
   public:
    Accessor() {};

    /**
     * @brief Accessor destructor function
     * @details Internally it handles the unwatching of a Node object it watched
     */
    ~Accessor() { deinit(); };

    /**
     * @brief Used to attempt to initialize the Accessor object
     * @details This function loads a value from the address and then attempts
     * to apply to memory protection to it.
     * Please see other implementation examples on how to apply memory protection.
     * It returns whether or not it is successful.
     * If nullptr is loaded then it returns true without calling memory
     * protection functions.
     * If success is returned, then node_ should be set equal to the loaded value
     *
     * @param address the address the reference to node was read from
     * @return whether or not memory protection was successfully applied
     */
    bool init(std::atomic<Node *> * const address);

    /**
     * @brief handles the unwatching of a Node object it watched
     * @details handles the unwatching of a Node object it watched
     */
    void deinit();


    bool isValid() { return node_ != nullptr; };
    T& value() { return node_->value(); };

    Node * node() { return node_; };

   private:
    Node * node_ {nullptr};
  };


  LinkedList() {};
  ~LinkedList();

  bool back(Accessor &access);
  bool front(Accessor &access);

  bool pop_front(Accessor &access);
  bool push_back(T &value);

  int64_t size() { return size_.load(); };
  bool full() { return false;};

  bool empty();

 private:
  std::atomic<int64_t> size_ {0};
  std::atomic<Node *> front_ {nullptr};
  std::atomic<Node *> back_ {nullptr};

};


}  // namespace lf
}  // namespace containers
}  // namespace tervel

#include <tervel/containers/lf/linked_list_queue/linkedlist_imp.h>

#endif  // __TERVEL_CONTAINERS_LF_LINKED_LIST_QUEUE_LINKED_LIST