#ifndef __TERVEL_CONTAINERS_LF_MCAS_BUFFER_MCAS_BUFFER_H__
#define __TERVEL_CONTAINERS_LF_MCAS_BUFFER_MCAS_BUFFER_H__

#include "tervel/util/info.h"
#include "tervel/util/descriptor.h"
#include "tervel/util/memory/hp/hazard_pointer.h"
#include "tervel/util/memory/rc/descriptor_util.h"

#include "tervel/algorithms/wf/mcas/mcas.h"

#include <atomic>


namespace tervel {
namespace containers {
namespace lf {
namespace mcas_buffer {



template<typename T>
class RingBuffer {
 public:
  explicit RingBuffer(size_t s)
    : buff_(new std::atomic<Node *>[s](c_not_value))
    , capacity_(s) {
      buff_[0].store(c_tail_value);
      buff_[1].store(c_head_value);
      head_.store(1);
      tail_.store(0);
    }

  ~RingBuffer() {
    for (size_t i = 0; i < capacity_; i++) {
      Node * cvalue = buff_[i].load();
      if (cvalue == c_tail_value) {
        continue;
      } else if (cvalue == c_head_value) {
        continue;
      } else if (cvalue == c_not_value) {
        continue;
      } else {
        // TODO(steven) delete descriptor.
      }
    }
  }

  bool is_empty() {
    size_t t = tail();
    size_t h = head();

    if (h > t+1) {
      return false;
    } else {
      return true;
    }
  }

  bool is_full() {
    size_t t = tail();
    size_t h = head();

    if (h+1 >= t+capacity_) {
      return false;
    } else {
      return true;
    }
  }

  bool enqueue(T value) {
    Node *node = tervel::util::memory::rc::get_descriptor<Node>(value);

    if (enqueue(node)) {
      return true;
    } else {
      util::memory::rc::free_descriptor(node);
      return false;
    }
  }

  bool dequeue(T &value) {
    Node *node = dequeue();
    if (node == c_not_value) {
      return false;
    } else {
      value = node->value();
      util::memory::rc::free_descriptor(node);
      return true;
    }
  };

  size_t capacity() {
    return capacity_;
  };

  void printQueue() {
    for (size_t i = 0; i < capacity_; i++) {
      Node * cvalue = buff_[i].load();
      if (cvalue == c_tail_value) {
        printf("[%u, TAIL(%p) ] ", i, cvalue);
      } else if (cvalue == c_head_value) {
        printf("[%u, HEAD(%p) ] ", i, cvalue);
      } else if (cvalue == c_not_value) {
        printf("[%u, NOT(%p) ] ", i, cvalue);
      } else {
        printf("[%u, VALU(%p)] ", i, cvalue);
      }
    }
    printf("\n");
  };

 private:
  class Node : public util::Descriptor {
   public:
    explicit Node(T val)
      : val_(val) {}

    T value() {
      return val_;
    }

    using util::Descriptor::complete;
    void * complete(void *value, std::atomic<void *> *address) {
      assert(false);
      return nullprt;
    }

    using util::Descriptor::get_logical_value;
    void * get_logical_value() {
      assert(false);
      return nullprt;
    }

   private:
    const T val_;
  };


  Node *at(size_t pos) {
    if (pos >= capacity_) {
      pos = pos % capacity_;
    }
    while (true) {
      Node *node = buff_[pos].load();

      if (util::memory::rc::is_descriptor_first(reinterpret_cast<void *>(
            node))) {
        util::memory::rc::remove_descriptor(reinterpret_cast<void *>(node),
            reinterpret_cast<std::atomic<void *> *>(address));
      } else {
        return node;
      }
    }
  }

  std::atomic<Node *> *address(size_t pos) {
    if (pos >= capacity_) {
      pos = pos % capacity_;
    }
    return &(buff_[pos]);
  }

  size_t head() {
    return head_.load();
  }

  size_t head(size_t i) {
    return head_.fetch_add(i);
  }

  size_t tail() {
    return tail_.load();
  }

  size_t tail(size_t i) {
    return tail_.fetch_add(i);
  }

  bool enqueue(Node * node) {
    if (is_full()) {
      return false;
    }

    h = head();

    while (true) {
      Node *current = at(h);
      if (current == c_head_value) {
        Node *next = at(h+1);

        if (next == c_tail_value) {
          return false;
        } else {
          tervel::mcas::MCAS<Node *> *mcas = new tervel::mcas::MCAS<Node *>(2);
          bool success = mcas->add_cas_triple(address(h), c_head_value, node);
          assert(success);

          success = mcas->add_cas_triple(address(h+1), c_not_value, c_head_value);
          assert(success);

          success = mcas->execute();
          mcas->safe_delete();

          if (success) {
            return true;
          } else {
            continue;
          }
        }

      } else if (current == c_not_value) {
        h = head();
      } else {
        h++;
      }
    }

  }

  Node * dequeue() {
    if (is_empty()) {
      return c_not_value;
    }

    t = tail();

    while (true) {
      Node *current = at(t);
      if (current == c_tail_value) {
        Node *next = at(t+1);

        if (next == c_head_value) {
          return c_not_value;
        } else if (next == c_tail_value) {
          t++;
        } else if (next == c_not_value) {
          t = tail();
        } else {  // some node *
          tervel::mcas::MCAS<Node *> *mcas = new tervel::mcas::MCAS<Node *>(2);
          bool success = mcas->add_cas_triple(address(h), c_tail_value, c_not_value);
          assert(success);

          success = mcas->add_cas_triple(address(h+1), next, c_tail_value);
          assert(success);

          success = mcas->execute();
          mcas->safe_delete();

          if (success) {
            return true;
          } else {
            continue;
          }
        }

      } else if (current == c_not_value) {
        t = tail();
      } else {
        t++;
      }
    }

  }

  const size_t capacity;
  std::atomic<long> head_;
  std::atomic<long> tail_;
  std::unique_ptr<std::atomic<Node *>[]> buff_;

  const Node * c_not_value = reinterpret_cast<Node *>(0x0L)
  const Node * c_head_value = reinterpret_cast<Node *>(0x10L)
  const Node * c_tail_value = reinterpret_cast<Node *>(0x20L)

};  // class RingBuffer


}  // namespace tervel
}  // namespace containers
}  // namespace lf
}  // namespace mcas_buffer

#endif  // __TERVEL_CONTAINERS_LF_MCAS_BUFFER_MCAS_BUFFER_H__
