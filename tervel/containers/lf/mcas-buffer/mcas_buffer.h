#ifndef __TERVEL_CONTAINERS_LF_MCAS_BUFFER_MCAS_BUFFER_H__
#define __TERVEL_CONTAINERS_LF_MCAS_BUFFER_MCAS_BUFFER_H__

#include <atomic>
#include <cstdint>
#include <algorithm>

#include <tervel/util/info.h>
#include <tervel/util/memory/rc/descriptor_util.h>
#include <tervel/util/descriptor.h>

#include <tervel/algorithms/wf/mcas/mcas.h>

namespace tervel {
namespace containers {
namespace lf {
namespace mcas_buffer {

template<typename T>
class Node : public util::Descriptor {
 public:
  explicit Node(T val)
    : val_(val) {}

  ~Node() {}

  T value() {
    return val_;
  };

  using util::Descriptor::complete;
  void * complete(void *current, std::atomic<void *> *address) {
    assert(false);
    return nullptr;
  };

  using util::Descriptor::get_logical_value;
  void * get_logical_value() {
    assert(false);
    return nullptr;
  };

 private:
  const T val_;
};  // class Node

template<typename T>
class RingBuffer {
 public:
  explicit RingBuffer(uint64_t s)
    : capacity_(s)
    , buff_(new std::atomic<Node<T> *>[s]) {
      buff_[0].store(c_tail_value);
      buff_[1].store(c_head_value);
      head_.store(1);
      tail_.store(0);
      for (uint64_t i = 2; i < capacity_; i++) {
        buff_[i].store(c_not_value);
      }
    };

  ~RingBuffer() {
    for (uint64_t i = 0; i < capacity_; i++) {
      Node<T> * cvalue = buff_[i].load();
      if (cvalue == c_tail_value) {
        continue;
      } else if (cvalue == c_head_value) {
        continue;
      } else if (cvalue == c_not_value) {
        continue;
      } else {
        delete cvalue;
      }
    }
  };


  bool enqueue(T value);
  bool dequeue(T &value);

  bool is_empty() {
    uint64_t t = tail();
    uint64_t h = head();

    if (h > t+1) {
      return false;
    } else {
      return true;
    }
  };

  bool is_full() {
    uint64_t t = tail();
    uint64_t h = head();

    if (h+1 >= t+capacity_) {
      return true;
    } else {
      return false;
    }
  };

  size_t capacity() {
    return capacity_;
  };

  void print_queue() {
    for (uint64_t i = 0; i < capacity_; i++) {
      Node<T> * cvalue = buff_[i].load();
      if (cvalue == c_tail_value) {
        printf("[%lu, TAIL(%p) ] ", i, cvalue);
      } else if (cvalue == c_head_value) {
        printf("[%lu, HEAD(%p) ] ", i, cvalue);
      } else if (cvalue == c_not_value) {
        printf("[%lu, NOT(%p) ] ", i, cvalue);
      } else {
        printf("[%lu, VAL(%p)] ", i, reinterpret_cast<void *>(cvalue->value()));
      }
    }
    printf("\n");
  };

 private:
  Node<T> *at(uint64_t pos) {
    if (pos >= capacity_) {
      pos = pos % capacity_;
    }
    while (true) {
      Node<T> *node = buff_[pos].load();

      if (util::memory::rc::is_descriptor_first(reinterpret_cast<void *>(
            node))) {
        util::memory::rc::remove_descriptor(reinterpret_cast<void *>(node),
            reinterpret_cast<std::atomic<void *> *>(&(buff_[pos])));
      } else {
        return node;
      }
    }
  }

  std::atomic<Node<T> *> *address(uint64_t pos) {
    if (pos >= capacity_) {
      pos = pos % capacity_;
    }
    return &(buff_[pos]);
  }

  uint64_t head() {
    return head_.load();
  }

  uint64_t head(uint64_t i) {
    return head_.fetch_add(i);
  }

  uint64_t tail() {
    return tail_.load();
  }

  uint64_t tail(uint64_t i) {
    return tail_.fetch_add(i);
  }

  bool enqueue(Node<T> * node) {
    if (is_full()) {
      return false;
    }

    uint64_t h = head();

    while (true) {
      Node<T> *current = at(h);
      if (current == c_head_value) {
        Node<T> *next = at(h+1);

        if (next == c_tail_value) {
          return false;
        } else {
          tervel::algorithms::wf::mcas::MCAS<Node<T> *> *mcas =
              new tervel::algorithms::wf::mcas::MCAS<Node<T> *>(2);
          bool success;
          success = mcas->add_cas_triple(address(h), c_head_value, node);
          assert(success);

          success = mcas->add_cas_triple(address(h+1), c_not_value, c_head_value);
          assert(success);

          success = mcas->execute();
          mcas->safe_delete();

          if (success) {
            head(1);
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
  }  /// enqueue

  Node<T> * dequeue() {
    if (is_empty()) {
      return c_not_value;
    }

    uint64_t t = tail();

    while (true) {
      Node<T> *current = at(t);
      if (current == c_tail_value) {
        Node<T> *next = at(t+1);

        if (next == c_head_value) {
          return c_not_value;
        } else if (next == c_tail_value) {
          t++;
        } else if (next == c_not_value) {
          t = tail();
        } else {  // some node *
          tervel::algorithms::wf::mcas::MCAS<Node<T> *> *mcas =
              new tervel::algorithms::wf::mcas::MCAS<Node<T> *>(2);
          bool success;
          success = mcas->add_cas_triple(address(t), c_tail_value, c_not_value);
          assert(success);

          success = mcas->add_cas_triple(address(t+1), next, c_tail_value);
          assert(success);

          success = mcas->execute();
          mcas->safe_delete();

          if (success) {
            tail(1);
            return next;
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
  }  // dequeue

  const uint64_t capacity_;
  std::atomic<uint64_t> head_;
  std::atomic<uint64_t> tail_;
  std::unique_ptr<std::atomic<Node<T> *>[]> buff_;

  Node<T> * c_not_value = reinterpret_cast<Node<T> *>(0x0L);
  Node<T> * c_head_value = reinterpret_cast<Node<T> *>(0x10L);
  Node<T> * c_tail_value = reinterpret_cast<Node<T> *>(0x20L);
};  // class RingBuffer



template<typename T>
bool RingBuffer<T>::enqueue(T value) {
  Node<T> *node = tervel::util::memory::rc::get_descriptor<Node<T>>(value);

  if (enqueue(node)) {
    return true;
  } else {
    util::memory::rc::free_descriptor(node);
    return false;
  }
};

template<typename T>
bool RingBuffer<T>::dequeue(T &value) {
  Node<T> *node = dequeue();
  if (node == c_not_value) {
    return false;
  } else {
    value = node->value();
    util::memory::rc::free_descriptor(node);
    return true;
  }
};

}  // namespace tervel
}  // namespace containers
}  // namespace lf
}  // namespace mcas_buffer

#endif  // __TERVEL_CONTAINERS_LF_MCAS_BUFFER_MCAS_BUFFER_H__
