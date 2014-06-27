#ifndef TERVEL_CONTAINER_WF_HASH_MAP_WFHM_HASHMAP_H
#define TERVEL_CONTAINER_WF_HASH_MAP_WFHM_HASHMAP_H

#include <stdlib.h>
#include <atomic>
#include <cmath>


// Todo:
// Add Memory management
// progress assurance
// comment stuff


namespace tervel {

namespace util {
  int round_to_next_power_of_two(uint64_t value) {
    double val = std::log2(value);
    int int_val = static_cast<int>(val);
    if (int_val < val) {
      int_val++;
    }
    return int_val;
  };
}

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
  HashMap(uint64_t capacity, uint64_t expansion_rate = 4)
    : primary_array_size_(tervel::util::round_to_next_power_of_two(capacity))
    , primary_array_pow_(std::log2(primary_array_size_))
    , secondary_array_size_(std::pow(2, expansion_rate))
    , secondary_array_pow_(expansion_rate)
    , primary_array_(new Location[primary_array_size_]) {}

  // TODO(implement).
  ~HashMap() {  }

  /**
   * TODO(steven): Provide general overview
   * @param  key   [description]
   * @param  value [description]
   * @return       [description]
   */
  bool find(Key key, Value &value);

  /**
   * TODO(steven): Provide general overview
   * @param  key   [description]
   * @param  value [description]
   * @return       [description]
   */
  bool insert(Key key, Value &value);

  /**
   * TODO(steven): Provide general overview
   * @param  key            [description]
   * @param  value_expected [description]
   * @param  value_new      [description]
   * @return                [description]
   */
  bool update(Key key, Value &value_expected, Value value_new);

  /**
   * TODO(steven): Provide general overview
   * @param  key            [description]
   * @param  value_expected [description]
   * @return                [description]
   */
  bool remove(Key key, Value &value_expected);

  /**
   * [size description]
   * @return [description]
   */
  size_t size() {
    return size_.load();
  };

 private:
  class Node;
  typedef std::atomic<Node *> Location;
  friend class Node;

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
      : internal_array_(new Location[len]) {}

    ~ArrayNode() {}  // TODO(implement)

    /**
     * TODO(steven): Provide general overview
     * @param  pos [description]
     * @return     [description]
     */
    Location *access(uint64_t pos) {
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
    std::unique_ptr<Location[]> internal_array_;
  };

  /**
   * TODO(steven): Provide general overview
   */
  class DataNode : public Node {
   public:
    explicit DataNode(Key k, Value v)
      : key_(k)
      , value_(v) {}

    ~DataNode() { }  // TODO(implement)

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
  };

  /**
   * TODO(steven): Provide general overview
   * @param loc           [description]
   * @param curr_value    [description]
   * @param next_position [description]
   */
  void expand_map(Location * loc, Node * curr_value, uint64_t next_position);

  /**
   * TODO(steven): Provide general overview
   * @param array    [description]
   * @param position [description]
   * @param current  [description]
   * @param depth    [description]
   * @param key      [description]
   * @param expand   [description]
   */
  void search(ArrayNode * &array, uint64_t &position, Node * &current,
    size_t &depth, Key &key, bool expand);

  /**
   * TODO(steven): Provide general overview
   * @param  k     [description]
   * @param  depth [description]
   * @return       [description]
   */
  uint64_t get_position(Key &k, size_t depth);

  const size_t primary_array_size_;
  const size_t primary_array_pow_;
  const size_t secondary_array_size_;
  const size_t secondary_array_pow_;

  std::atomic<uint64_t> size_;

  std::unique_ptr<Location[]> primary_array_;
};

template<class Key, class Value, class Functor>
bool HashMap<Key, Value, Functor>::
find(Key key, Value &value) {
  Functor functor;
  key = functor.hash(key);

  uint64_t position = get_position(key, 0);

  Node *curr_value = primary_array_[position].load();

  if (curr_value->is_array()) {
    size_t depth = 0;
    ArrayNode * array_node = reinterpret_cast<ArrayNode *>(curr_value);
    search(array_node, position, curr_value, depth, key, false);
  }

  assert(curr_value == nullptr || curr_value->is_data());
  if (curr_value == nullptr) {
    return false;
  } else {  // it is a data node
    DataNode * data_node = reinterpret_cast<DataNode *>(curr_value);
    assert(functor.key_equals(data_node->key_, key));
    value = data_node->value_;
    return true;
  }
}  // find

template<class Key, class Value, class Functor>
bool HashMap<Key, Value, Functor>::
insert(Key key, Value &value) {
  Functor functor;
  key = functor.hash(key);

  DataNode * new_node = new DataNode(key, value);

  size_t depth = 0;
  uint64_t position = get_position(key, depth);

  Location *loc = &(primary_array_[position]);
  Node *curr_value = loc->load();

  while (true) {
    if (curr_value->is_array()) {
      ArrayNode * array_node = reinterpret_cast<ArrayNode *>(curr_value);
      search(array_node, position, curr_value, depth, key, true);
      loc = array_node->access(position);
    }

    assert(curr_value == nullptr || curr_value->is_data());
    if (curr_value == nullptr) {
      if (loc->compare_exchange_strong(curr_value, new_node)) {
        size_.fetch_add(1);
        return true;
      } else {
        continue;
      }
    } else {  // it is a data node
      DataNode * data_node = reinterpret_cast<DataNode *>(curr_value);
      assert(functor.key_equals(data_node->key_, key));
      value = data_node->value_;

      // delete new_node;
      return false;
    }
  }
}  // insert

template<class Key, class Value, class Functor>
bool HashMap<Key, Value, Functor>::
update(Key key, Value &value_expected, Value value_new) {
  Functor functor;
  key = functor.hash(key);

  DataNode * new_node = new DataNode(key, value_new);

  size_t depth = 0;
  uint64_t position = get_position(key, depth);

  Location *loc = &(primary_array_[position]);
  Node *curr_value = loc->load();

  while (true) {
    if (curr_value->is_array()) {
      ArrayNode * array_node = reinterpret_cast<ArrayNode *>(curr_value);
      search(array_node, position, curr_value, depth, key, true);
      loc = array_node->access(position);
    }

    assert(curr_value == nullptr || curr_value->is_data());
    if (curr_value == nullptr) {
      // delete new_node;
      return false;
    } else {  // it is a data node
      DataNode * data_node = reinterpret_cast<DataNode *>(curr_value);
      assert(functor.key_equals(data_node->key_, key));

      if (functor.value_equals(value_expected, data_node->value_)) {
        if (loc->compare_exchange_strong(curr_value, new_node)) {
          // delete data_node;
          return true;
        } else {
          continue;
        }
      } else {
        value_expected = data_node->value_;
        // delete new_node;
        return false;
      }
    }
  }
}  // update


template<class Key, class Value, class Functor>
bool HashMap<Key, Value, Functor>::
remove(Key key, Value &expected) {
  Functor functor;
  key = functor.hash(key);

  size_t depth = 0;
  uint64_t position = get_position(key, depth);

  Location *loc = &(primary_array_[position]);
  Node *curr_value = loc->load();

  while (true) {
    if (curr_value->is_array()) {
      ArrayNode * array_node = reinterpret_cast<ArrayNode *>(curr_value);
      search(array_node, position, curr_value, depth, key, true);
      loc = array_node->access(position);
    }

    assert(curr_value == nullptr || curr_value->is_data());
    if (curr_value == nullptr) {
      return false;
    } else {  // it is a data node
      DataNode * data_node = reinterpret_cast<DataNode *>(curr_value);
      assert(functor.key_equals(data_node->key_, key));

      if (functor.value__equals(expected, data_node->value_)) {
        if (loc->compare_exchange_strong(curr_value, nullptr)) {
          // delete data_node;
          size_.fetch_add(-1);
          return true;
        } else {
          continue;
        }
      } else {
        expected = data_node->value_;
        return false;
      }
    }
  }
}  // remove

template<class Key, class Value, class Functor>
void HashMap<Key, Value, Functor>::
search(ArrayNode * &array, uint64_t &position, Node * &curr_value,
    size_t &depth, Key &key, bool expand) {
  Functor functor;
  // First update the depth
  depth++;

  while (true) {
    position = get_position(key, depth);
    Location *loc = array->access(position);
    curr_value = loc->load();

    if (curr_value == nullptr) {
      // Null is a terminating condition
      return;
    } else if (curr_value->is_array()) {
      // Need to examine the next array, so update array reference and the depth
      // count. Position and curr_value will be update at the start of the loop
      array = reinterpret_cast<ArrayNode *>(curr_value);
      depth++;
      continue;
    } else {  // its a data_node
      DataNode * data_node = reinterpret_cast<DataNode *>(curr_value);
      if (functor.key_equals(data_node->key_, key)) {
        // A key matching data node is a terminating condition
        return;
      } else if (expand == false) {
        // This occurs when search is called by 'find', since it is data node
        // that does not match the key, then curr_valu == nullptr indicates the
        // key is not in the hash map
        curr_value = nullptr;
        return;
      } else {
        // A key miss match data node is found, need to expand and re-examine
        // the current value.
        const uint64_t next_position =  get_position(data_node->key_, depth+1);
        expand_map(loc, curr_value, next_position);
        continue;
      }
    }  // else its a data node
  }  // while
}  // search

template<class Key, class Value, class Functor>
void  HashMap<Key, Value, Functor>::
expand_map(Location * loc, Node * curr_value, uint64_t next_position) {
  ArrayNode * array_node = new ArrayNode(secondary_array_size_);
  array_node->access(next_position)->store(curr_value);
  if (loc->compare_exchange_strong(curr_value, array_node)) {
    return;
  } else {
    // delete array_node;
  }
}  // expand

template<class Key, class Value, class Functor>
uint64_t HashMap<Key, Value, Functor>::
get_position(Key &key, size_t depth) {
  const uint64_t *long_array = static_cast<uint64_t *>(&key);
  const size_t max_length = sizeof(Key) / (64 / 8);
  if (depth == 0) {
    // We need the first 'primary_array_pow_' bits
    assert(primary_array_pow_ < 64);

    uint64_t position = long_array[0] >> (64 - primary_array_pow_);
    return position;
  } else {
    assert(false);  // Todo implement
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
    // TODO(steven): add 0 padding to fill extra bits if the bits don't divide evenly.
    if (start_idx == end_idx) {
      uint64_t value = long_array[start_idx];
      value = value << start_idx_offset;
      value = value >> (64 - secondary_array_pow_);
      return value;
    } else {
      uint64_t value = long_array[start_idx];
      value = value << start_idx_offset;
      value = value >> (64 - secondary_array_pow_);
      value = value << end_idx_offset;

      uint64_t value2 = long_array[end_idx];
      value2 = value2 >> (64 - end_idx_offset);

      return (value | value2);
    }
  }
}  // get_position

}  // namespace wf
}  // namespace containers
}  // namespace tervel

#endif  // TERVEL_CONTAINER_WF_HASH_MAP_WFHM_HASHMAP_H
