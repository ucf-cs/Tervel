#ifndef TERVEL_WFHM_HASHMAP_H
#define TERVEL_WFHM_HASHMAP_H

#include <stdlib.h>
#include <atomic>
#include <cmath>

#include "tervel/wf-hash-map/node.h"
#include "tervel/wf-hash-map/pair_node.h"
#include "tervel/wf-hash-map/array_node.h"


/**
 * A default functor implementation
 *
 */
template<class Key, class Value>
typedef struct {
  Key hash(Key k) {
    return k;
  }

  bool value_equals(Value a, Value b) {
    return a == b;
  }

  bool key_equals(Key a, key b) {
    return a == b;
  }
} default_functor;


/**
 * A wait-free hash map implementation.
 *
 * functor should have the following functions:
 *   -Key hash(Key k) (where hash(a) == (hash(b) implies a == b
 *   -bool value_equals(Value a, Value b)
 *   -bool key_equals (Key a, Key b)
 *       Important Note: the hashed value of keys will be passed in.
 *
 */
template<class Key, class Value, class functor = default_functor>
class HashMap {
 public:
  HashMap(uint64_t capacity, uint64_t expansion_rate = 4)
    : primary_array_size_(round_to_next_power_of_two(capacity))
    , primary_array_pow_(log2(primary_array_size_))
    , secondary_array_size_(std::pow(2, expansion_rate))
    , secondary_array_pow_(expansion_rate)
    , primary_array_(new Location[primary_array_size_]) {}

  bool find(Key key, const Value &value);
  bool insert(Key key, const Value &value);
  bool update(Key key, const Value &value_expected, Value value_new);
  bool remove(Key key, const Value &value_expected);

 private:
  class Hash {
   public:
    uint64_t get_position(size_t depth) {
      const uint64_t *long_array = static_cast<uint64_t *>(&data);
      if (depth == 0) {
        // We need the first 'primary_array_pow_' bits
        assert(primary_array_pow_ < 64);

        uint64_t position = long_array[0] >> (64 - primary_array_pow_);
        return position;
      } else {
        // Start of Relevant bits:
        //  (depth-1)*secondary_array_pow_ + primary_array_pow_
        // End of Relevant bits
        //  (depth)*secondary_array_pow_ + primary_array_pow_
        assert(false);  // Todo implement
      }
    };
   private:
    char data[sizeof(Key)];
  };

  class Node {
    Node() {}
    virtual ~Node() {}

    virtual bool is_array() = 0;
    virtual bool is_data() = 0;
  };

  class ArrayNode : public Node {
   public:
    explicit ArrayNode(uint64_t len)
      : internal_array_(new Location[len]) {}


    Location *access(uint64_t pos) {
      return &(internal_array_[pos]);
    }

    bool is_array() {
      return true;
    }
    bool is_data() {
      return false;
    }

   private:
    std::unique_ptr<Location[]> internal_array_;
  };

  class DataNode : public Node {
   public:
    explicit DataNode(Key k, Value v)
      : key_(k)
      , value_(v) {}

    bool is_array() {
      return false;
    }
    bool is_data() {
      return true;
    }

    Key key_;
    Value value_;
  };

  void expand(Location * loc, Node * curr_value, uint64_t next_position);
  void search(ArrayNode * &array, const uint64_t &position, Node * &current,
    const size_t &depth, Key &key, bool expand);



  const size_t primary_array_size_;
  const size_t primary_array_pow_;
  const size_t secondary_array_size_;
  const size_t secondary_array_pow_;

  typedef std::atomic<Node *> Location;
  std::unique_ptr<Location[]> primary_array_;
};

template<class Key, class Value, class functor>
bool HashMap<Key, Value, functor>::
find(Key key, const Value &value) {
  key = functor.hash(key);

  const Hash *hashed = static_cast<Hash *>(&key);
  uint64_t position = hashed->get_position(0);

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
};

template<class Key, class Value, class functor>
bool HashMap<Key, Value, functor>::
insert(Key key, const Value &value) {
  key = functor.hash(key);
  const Hash *hashed = static_cast<Hash *>(&key);

  DataNode * new_node = new DataNode(key, value);

  size_t depth = 0;
  uint64_t position = hashed->get_position(depth);

  Location *loc = &(primary_array_[position]);
  Node *curr_value = loc.load();

  while (true) {
    if (curr_value->is_array()) {
      ArrayNode * array_node = reinterpret_cast<ArrayNode *>(curr_value);
      search(array_node, position, curr_value, depth, key, true);
      loc = array_node->access(position);
    }

    assert(curr_value == nullptr || curr_value->is_data());
    if (curr_value == nullptr) {
      if (loc->compare_exchange_strong(curr_value, new_node)) {
        return true;
      } else {
        continue;
      }
    } else {  // it is a data node
      DataNode * data_node = reinterpret_cast<DataNode *>(curr_value);
      assert(functor.key_equals(data_node->key_, key));
      value = data_node->value_;

      delete new_node;
      return false;
    }
  }
};

template<class Key, class Value, class functor>
bool HashMap<Key, Value, functor>::
update(Key key, const Value &value_expected, Value value_new) {
  key = functor.hash(key);
  const Hash *hashed = static_cast<Hash *>(&key);

  DataNode * new_node = new DataNode(key, value_new);

  size_t depth = 0;
  uint64_t position = hashed->get_position(depth);

  Location *loc = &(primary_array_[position]);
  Node *curr_value = loc.load();

  while (true) {
    if (curr_value->is_array()) {
      ArrayNode * array_node = reinterpret_cast<ArrayNode *>(curr_value);
      search(array_node, position, curr_value, depth, key, true);
      loc = array_node->access(position);
    }

    assert(curr_value == nullptr || curr_value->is_data());
    if (curr_value == nullptr) {
      delete new_node;
      return false;
    } else {  // it is a data node
      DataNode * data_node = reinterpret_cast<DataNode *>(curr_value);
      assert(functor.key_equals(data_node->key_, key));

      if (functor->value__equals(value_expected, data_node->value_) {
        if (loc->compare_exchange_strong(curr_value, new_node)) {
          delete data_node;
          return true;
        } else {
          continue;
        }
      } else {
        value_expected = data_node->value_;
        delete new_node;
        return false;
      }
    }
  }
};


template<class Key, class Value, class functor>
bool HashMap<Key, Value, functor>::
remove(Key key, const Value &expected) {
  key = functor.hash(key);
  const Hash *hashed = static_cast<Hash *>(&key);

  size_t depth = 0;
  uint64_t position = hashed->get_position(depth);

  Location *loc = &(primary_array_[position]);
  Node *curr_value = loc.load();

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

      if (functor->value__equals(expected, data_node->value_) {
        if (loc->compare_exchange_strong(curr_value, nullptr)) {
          delete data_node;
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
};

template<class Key, class Value, class functor>
void HashMap<Key, Value, functor>::
search(ArrayNode * &array, const uint64_t &position, Node * &curr_value,
    const size_t &depth, Key &key, bool expand) {
  // First update the depth
  depth++;
  const Hash *hashed = static_cast<Hash *>(&key);

  while (true) {
    position = hashed->get_position(depth);
    Location *loc = array->access(position);
    curr_value = loc.load();

    if (curr_value == nullptr) {
      // Null is a terminating condition
      return;
    } else if (curr_value->is_array() {
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
        const Hash *hashed_other = static_cast<Hash *>(&(data_node->key_));
        const uint64_t next_position =  hashed_other->get_position(depth+1);
        expand(loc, curr_value, next_position);
        continue;
      }
    }
  }  // while
}  // search

template<class Key, class Value, class functor>
void  HashMap<Key, Value, functor>::
expand(Location * loc, Node * curr_value, uint64_t next_position) {
  ArrayNode * array_node = new ArrayNode(secondary_array_size_);
  array_node->access(next_position)->store(curr_value);
  if (loc->compare_exchange_strong(curr_value, array_node)) {
    return;
  } else {
    delete array_node;
  }
}
