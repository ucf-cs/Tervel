#ifndef TERVEL_CONTAINER_WF_HASH_MAP_WFHM_HASHMAP_H
#define TERVEL_CONTAINER_WF_HASH_MAP_WFHM_HASHMAP_H

#include <stdlib.h>
#include <atomic>
#include <cmath>

#include <tervel/util/info.h>
#include <tervel/util/memory/hp/hp_element.h>
#include <tervel/util/memory/hp/hazard_pointer.h>
#include <tervel/util/progress_assurance.h>

// TODO(Steven):
//
// Document code
//
// Implement Progress Assurance.
//
// Test get_position function on keys > 64 bits
// Stronger Correctness Tests
//
namespace tervel {

namespace containers {
namespace wf {
/**
 * A default Functor implementation
 *
 */
template<class Key, class Value>
struct default_functor {
  Key hash(Key k) {
    return k;
  }

  bool value_equals(Value a, Value b) {
    return a == b;
  }

  bool key_equals(Key a, Key b) {
    return a == b;
  }
};



/**
 * A wait-free hash map implementation.
 *
 * TODO(steven): Provide general overview
 *
 * Functor should have the following functions:
 *   -Key hash(Key k) (where hash(a) == (hash(b) implies a == b
 *   -bool value_equals(Value a, Value b)
 *   -bool key_equals (Key a, Key b)
 *       Important Note: the hashed value of keys will be passed in.
 *
 */
template< class Key, class Value, class Functor = default_functor<Key, Value> >
class HashMap {
 public:
  HashMap(uint64_t capacity, uint64_t expansion_rate = 3)
    : primary_array_size_(tervel::util::round_to_next_power_of_two(capacity))
    , primary_array_pow_(std::log2(primary_array_size_))
    , secondary_array_size_(std::pow(2, expansion_rate))
    , secondary_array_pow_(expansion_rate)
    , primary_array_(new Location[primary_array_size_]()) { }

  /**
   * Not Thread Safe!
   * May create a stack as  deep as terversal
   *   If it is a node type then the node will be freed
   *   If it is an array node then the destructor of the array node will free
   *     any data nodes or array nodes that are referenced it.
   */
  ~HashMap() {
    for (size_t i = 0; i < primary_array_size_; i++) {
      Node * temp = primary_array_[i].load();
      if (temp != nullptr) {
        delete temp;
      }
    }
  }  // ~ HashMap

  class ValueAccessor;

  /**
   * TODO(steven): Provide general overview
   * @param  key   [description]
   * @param  value [description]
   * @return       [description]
   */
  bool at(Key key, ValueAccessor &va);

  /**
   * TODO(steven): Provide general overview
   * @param  key   [description]
   * @param  value [description]
   * @return       [description]
   */
  bool insert(Key key, Value value);

  /**
   * TODO(steven): Provide general overview
   * @param  key            [description]
   * @return                [description]
   */
  bool remove(Key key);

  /**
   * [size description]
   * @return [description]
   */
  size_t size() {
    return size_.load();
  };


  class ValueAccessor {
   public:
    ValueAccessor()
      : access_count_(nullptr)
      , value_(nullptr) {}

    void init(Value * value, std::atomic<int64_t> *access_count) {
      if (access_count_) {
        // In case they reuse the object
        access_count_->fetch_add(-1);
      }

      value_ =value;
      access_count_ = access_count;
    }

    ~ValueAccessor() {
      if (access_count_) {
        access_count_->fetch_add(-1);
      }
    }

    Value *value() {
      return value_;
    }

    std::atomic<Value> *atomic_value() {
      return (std::atomic<Value> *)(value_);
    }

    bool valid() {
      return (value_ != nullptr);
    }

   private:
    std::atomic<int64_t> *access_count_;
    Value *value_{nullptr};

  };
  /**
   * TODO(steven): Provide general overview
   * @param  k     [description]
   * @param  depth [description]
   * @return       [description]
   */
  uint64_t get_position(Key &k, size_t depth);

  uint64_t max_depth();
  void print_key(Key &key);

 private:
  class Node;
  typedef std::atomic<Node *> Location;
  friend class Node;
  friend class ForceExapndOp;
  /**
   * TODO(steven): Provide general overview
   */
  class Node {
   public:
    Node() {}
    virtual ~Node() {}

    /**
     * TODO(steven): Provide general overview
     * @return [description]
     */
    virtual bool is_array() = 0;

    /**
     * TODO(steven): Provide general overview
     * @return [description]
     */
    virtual bool is_data() = 0;
  };

  /**
   * TODO(steven): Provide general overview
   */
  class ArrayNode : public Node {
   public:
    explicit ArrayNode(uint64_t len)
      : len_(len)
      , internal_array_(new Location[len]()) {
        for (size_t i = 0; i < len_; i++) {
          internal_array_[i].store(nullptr);
        }
      }

    /**
     * See Notes on hash map destructor.
     */
    ~ArrayNode() {
      for (size_t i = 0; i < len_; i++) {
        Node * temp = internal_array_[i].load();
        if (temp != nullptr) {
          delete temp;
        }
      }
    }  // ~ArrayNode


    /**
     * TODO(steven): Provide general overview
     * @param  pos [description]
     * @return     [description]
     */
    Location *access(uint64_t pos) {
      assert(pos < len_ && pos >=0);
      return &(internal_array_[pos]);
    }

    /**
     * TODO(steven): Provide general overview
     * @return [description]
     */
    bool is_array() {
      return true;
    }

    /**
     * TODO(steven): Provide general overview
     * @return [description]
     */
    bool is_data() {
      return false;
    }

   private:
    uint64_t len_;
    std::unique_ptr<Location[]> internal_array_;
  };

  /**
   * TODO(steven): Provide general overview
   */
  class DataNode : public Node, public tervel::util::memory::hp::Element {
   public:
    explicit DataNode(Key k, Value v)
      : key_(k)
      , value_(v)
      , access_count_(0) {}

    ~DataNode() { }

    /**
     * TODO(steven): Provide general overview
     * @return [description]
     */
    bool is_array() {
      return false;
    }

    /**
     * TODO(steven): Provide general overview
     * @return [description]
     */
    bool is_data() {
      return true;
    }

    Key key_;
    Value value_;
    std::atomic<int64_t> access_count_;
  };

  class ForceExpandOp : util::OpRecord {
    public:
      ForceExpandOp(HashMap *map, Location *loc, size_t depth)
       : map_(map)
       , loc_(loc)
       , depth_(depth){}

      void help_complete() {
        if (depth_ >= map_->max_depth()) {
            return;
        }
        Node *value = loc_->load();
        while (true) {
          if (!map_->hp_watch_and_get_value(loc_,value)) {
             continue;
          } else if (value == nullptr || value->is_data()) {
            map_->expand_map(loc_, value, depth_);
            value = loc_->load();
          } else {
            break;
          }
        }
      };

    private:
      friend class HashMap;
      HashMap *map_{nullptr};
      Location *loc_{nullptr};
      size_t depth_{0};
   };

  /**
   * TODO(steven): Provide general overview
   * @param loc           [description]
   * @param curr_value    [description]
   * @param next_position [description]
   */
  void expand_map(Location * loc, Node * curr_value, size_t depth);

  bool hp_watch_and_get_value(Location * loc, Node * &value);
  void hp_unwatch();

  const size_t primary_array_size_;
  const size_t primary_array_pow_;
  const size_t secondary_array_size_;
  const size_t secondary_array_pow_;

  std::atomic<uint64_t> size_;

  std::unique_ptr<Location[]> primary_array_;
};  // class wf hash map

template<class Key, class Value, class Functor>
bool HashMap<Key, Value, Functor>::
hp_watch_and_get_value(Location * loc, Node * &value) {
  std::atomic<void *> *temp_address =
      reinterpret_cast<std::atomic<void *> *>(loc);

  void * temp = temp_address->load();

  if (temp == nullptr) {
    value = nullptr;
    return true;
  }

  bool is_watched = tervel::util::memory::hp::HazardPointer::watch(
        tervel::util::memory::hp::HazardPointer::SlotID::SHORTUSE,
        temp, temp_address, temp);

  if (is_watched) {
    value = reinterpret_cast<Node *>(temp);
  }
  return is_watched;
}  // hp_watch_and_get_value

template<class Key, class Value, class Functor>
void HashMap<Key, Value, Functor>::
hp_unwatch() {
  tervel::util::memory::hp::HazardPointer::unwatch(
          tervel::util::memory::hp::HazardPointer::SlotID::SHORTUSE);
}  // hp_unwatch


template<class Key, class Value, class Functor>
bool HashMap<Key, Value, Functor>::
at(Key key, ValueAccessor &va) {
  tervel::util::ProgressAssurance::check_for_announcement();
  Functor functor;
  key = functor.hash(key);

  size_t fcount = 0;
  size_t depth = 0;
  uint64_t position = get_position(key, depth);
  Location *loc = &(primary_array_[position]);
  Node *curr_value;
  while (true) {
   if (fcount++ > util::ProgressAssurance::MAX_FAILURES + 1) {
      ForceExpandOp *op = new ForceExpandOp(this, loc, depth);
      util::ProgressAssurance::make_announcement(
            reinterpret_cast<tervel::util::OpRecord *>(op));
      fcount = 0;
      continue;
   }
   if (!hp_watch_and_get_value(loc, curr_value)) {
      continue;
    } else if (curr_value == nullptr) {
      return false;
    } else if (curr_value->is_array()) {
      ArrayNode * array_node = reinterpret_cast<ArrayNode *>(curr_value);
      depth++;
      position = get_position(key, depth);
      loc = array_node->access(position);
      continue;
    } else {
      assert(curr_value->is_data());
      DataNode * data_node = reinterpret_cast<DataNode *>(curr_value);

      bool op_res = false;
      if (functor.key_equals(data_node->key_, key)) {
        int64_t res = data_node->access_count_.fetch_add(1);
        if (res >= 0) {  // its not deleted.
          va.init(&(data_node->value_), &(data_node->access_count_));
          op_res = true;
        }
      }

      hp_unwatch();
      return op_res;
    }
  }  // while true
}  // at


template<class Key, class Value, class Functor>
bool HashMap<Key, Value, Functor>::
insert(Key key, Value value) {
  tervel::util::ProgressAssurance::check_for_announcement();
  Functor functor;
  key = functor.hash(key);

  DataNode * new_node = new DataNode(key, value);

  size_t fcount = 0;
  size_t depth = 0;
  uint64_t position = get_position(key, depth);

  Location *loc = &(primary_array_[position]);
  Node *curr_value;

  bool op_res;
  while (true) {
    if (fcount++ > util::ProgressAssurance::MAX_FAILURES + 1) {
      ForceExpandOp *op = new ForceExpandOp(this, loc, depth);
      util::ProgressAssurance::make_announcement(
            reinterpret_cast<tervel::util::OpRecord *>(op));
      fcount = 0;
      continue;
    }
    if (!hp_watch_and_get_value(loc, curr_value)) {
      continue;
    } else if (curr_value == nullptr) {
      if (loc->compare_exchange_strong(curr_value, new_node)) {
        size_.fetch_add(1);
        op_res = true;
        break;
      }
    } else if (curr_value->is_array()) {
      ArrayNode * array_node = reinterpret_cast<ArrayNode *>(curr_value);
      depth++;
      position = get_position(key, depth);
      loc = array_node->access(position);
    } else {  // it is a data node
      assert(curr_value->is_data());
      DataNode * data_node = reinterpret_cast<DataNode *>(curr_value);

      if (data_node->access_count_.load() < 0) {
        if (loc->compare_exchange_strong(curr_value, new_node)) {
          data_node->safe_delete();
          size_.fetch_add(1);
          op_res = true;
          break;
        }
      } else if (functor.key_equals(data_node->key_, key)) {
        op_res = false;
        break;
      } else {
        // Key differs, needs to expand...
        expand_map(loc, curr_value, depth);
      }   // else key differs
    }  // else it is a data node
  }  // while true

  hp_unwatch();

  if (!op_res) {
    assert(loc->load() != new_node);
    delete new_node;
  }
  return op_res;
}  // insert


template<class Key, class Value, class Functor>
bool HashMap<Key, Value, Functor>::
remove(Key key) {
  tervel::util::ProgressAssurance::check_for_announcement();
  Functor functor;
  key = functor.hash(key);

  size_t depth = 0;
  uint64_t position = get_position(key, depth);

  Location *loc = &(primary_array_[position]);
  Node *curr_value;

  size_t fcount = 0;
  bool op_res = false;
  while (true) {
    if (fcount++ > util::ProgressAssurance::MAX_FAILURES +1) {
      ForceExpandOp *op = new ForceExpandOp(this, loc, depth);
      util::ProgressAssurance::make_announcement(
            reinterpret_cast<tervel::util::OpRecord *>(op));
      fcount = 0;
      continue;
    }
    if (!hp_watch_and_get_value(loc, curr_value)) {
      continue;
    } else if (curr_value == nullptr) {
      op_res = false;
      break;
    } else if (curr_value->is_array()) {
      ArrayNode * array_node = reinterpret_cast<ArrayNode *>(curr_value);
      depth++;
      position = get_position(key, depth);
      loc = array_node->access(position);
      continue;
    } else {  // it is a data node
      assert(curr_value->is_data());
      DataNode *data_node = reinterpret_cast<DataNode *>(curr_value);
      int64_t temp_expected = 0;
      if (functor.key_equals(data_node->key_, key) &&
          data_node->access_count_.compare_exchange_strong(temp_expected,
                  -1*tl_thread_info->get_num_threads())
                                                      ) {
        op_res = true;
        size_.fetch_add(-1);
        // data_node is a key match, value match, and we set it to dead
        if (loc->compare_exchange_strong(curr_value, nullptr)) {
            assert(loc->load() != data_node);
            assert(data_node->access_count_.load() < 0);
            data_node->safe_delete();
        } else {
          // It is logically deleted, and some other thread will/has already
          // will delete it.
        }
      }
      break;
    }  // it is a data node
  }  // while

  hp_unwatch();
  return op_res;
}  // remove



template<class Key, class Value, class Functor>
void  HashMap<Key, Value, Functor>::
expand_map(Location * loc, Node * curr_value, size_t depth) {
  DataNode *data_node = reinterpret_cast<DataNode *>(curr_value);
  uint64_t next_position = 0;
  ArrayNode * array_node = new ArrayNode(secondary_array_size_);
  if (curr_value != nullptr) {
    assert(curr_value->is_data());
    next_position = get_position(data_node->key_, depth+1);
    array_node->access(next_position)->store(curr_value);
  }
  assert(array_node->is_array());

  if (loc->compare_exchange_strong(curr_value, array_node)) {
    return;
  } else {
    assert(loc->load() != array_node);
    array_node->access(next_position)->store(nullptr);
    delete array_node;
  }
}  // expand

template<class Key, class Value, class Functor>
uint64_t HashMap<Key, Value, Functor>::
get_position(Key &key, size_t depth) {
  const uint64_t *long_array = reinterpret_cast<uint64_t *>(&key);
  const size_t max_length = sizeof(Key) / (64 / 8);

  assert(depth <= max_depth());
  if (depth == 0) {
    // We need the first 'primary_array_pow_' bits
    assert(primary_array_pow_ < 64);

    uint64_t position = long_array[0] >> (64 - primary_array_pow_);

    assert(position < primary_array_size_);
    return position;
  } else {
    const int start_bit_offset = (depth-1)*secondary_array_pow_ +
        primary_array_pow_;  // Inclusive
    const int end_bit_offset = (depth)*secondary_array_pow_ +
        primary_array_pow_;   // Not inclusive

    const size_t start_idx = start_bit_offset / 64;
    const size_t start_idx_offset = start_bit_offset % 64;
    const size_t end_idx = end_bit_offset / 64;
    const size_t end_idx_offset = end_bit_offset % 64;

    assert(start_idx == end_idx || start_idx + 1 == end_idx);
    assert(end_idx <= max_length);

    // TODO(steven): add 0 padding to fill extra bits if the bits don't
    // divide evenly.
    if (start_idx == end_idx) {
      uint64_t value = long_array[start_idx];
      value = value << start_idx_offset;
      value = value >> (64 - secondary_array_pow_);

      assert(value < secondary_array_size_);
      return value;
    } else {
      uint64_t value = long_array[start_idx];
      value = value << start_idx_offset;
      value = value >> (64 - secondary_array_pow_ + end_idx_offset);
      value = value << (end_idx_offset);


      uint64_t value2;
      if (end_idx == max_length) {
        value2 = 0;
      } else {
        value2 = long_array[end_idx];
      }
      value2 = value2 >> (64 - end_idx_offset);

      uint64_t position = (value | value2);
      assert(position < secondary_array_size_);
      return position;
    }
  }
}  // get_position

template<class Key, class Value, class Functor>
uint64_t HashMap<Key, Value, Functor>::
max_depth() {
  uint64_t max_depth = sizeof(Key)*8;
  max_depth -= primary_array_pow_;
  max_depth = std::ceil(max_depth / secondary_array_pow_);
  max_depth++;

  return max_depth;
}

template<class Key, class Value, class Functor>
void HashMap<Key, Value, Functor>::
print_key(Key &key) {
  size_t max_depth_ = max_depth();
  std::cout << "K(" << key << ") :";
  for (size_t i = 0; i <= max_depth_; i++) {
    uint64_t temp = get_position(key, i);
    std::cout << temp << "-";
  }
  std::cout << "\n" << std::endl;
}  // print_key

}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINER_WF_HASH_MAP_WFHM_HASHMAP_H
