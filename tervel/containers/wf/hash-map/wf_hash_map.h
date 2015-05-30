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
#ifndef TERVEL_CONTAINER_WF_HASH_MAP_WFHM_HASHMAP_H
#define TERVEL_CONTAINER_WF_HASH_MAP_WFHM_HASHMAP_H


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
 *   -bool key_equals (Key a, Key b)
 *       Important Note: the hashed value of keys will be passed in.
 *
 */
template< class Key, class Value, class Functor = default_functor<Key, Value> >
class HashMap {
 public:
  class ValueAccessor;

  HashMap(uint64_t capacity, uint64_t expansion_rate = 3)
    : primary_array_size_(tervel::util::round_to_next_power_of_two(capacity))
    , primary_array_pow_(std::log2(primary_array_size_))
    , secondary_array_size_(std::pow(2, expansion_rate))
    , secondary_array_pow_(expansion_rate)
    , primary_array_(new Location[primary_array_size_]()) { }

  /**
   * Not Thread Safe!
   * May create a very large stack!
   *   Note: should implement a better way...
   *   If it is a node type then the node will be freed
   *   If it is an array node then the destructor of the array node will free
   *     any nodes that are referenced it. This may cause more array node's to
   *     be freed. Stack can reach max_depth() in size.
   */
  ~HashMap() {
    for (size_t i = 0; i < primary_array_size_; i++) {
      Node * temp = primary_array_[i].load();
      if (temp != nullptr) {
        delete temp;
      }
    }
  }  // ~ HashMap



  /**
   * This function returns true and initializes the passed ValueAccessor if the
   * key exists in the hash map. Initializing the ValueAccessor consists of
   * assigning storing a reference to the associated value and a reference to
   * the access counter. The access counter will have been increased by one.
   * Upon the destruction or re-initialization of the ValueAccessor, the access
   * counter will be decremented.
   *
   * The sequential complexity of this operation is O(max_depth()).
   *
   * @param key: the key to look up
   * @param va: the location to store the address of the value/access_counter
   * @return whether or not the key is present
   */
  bool at(Key key, ValueAccessor &va);

  /**
   * This function returns true if the key value pair was successfully inserted.
   * Otherwise it returns false.
   *
   * A key can fail to insert in the event the key is already present.
   *
   * The sequential complexity of this operation is O(max_depth()).
   *
   * @param key: The key to insert
   * @param value: The key's associated value
   * @return whether or not the the key/value was inserted
   */
  bool insert(Key key, Value value);

  /**
   * Attempts to remove a key/value pair from the hash map
   * Returns false in the event the key is not in the hash map or if the
   * access_counter is non-zero.
   *
   * @param key: The key to resume
   * @return where or not the key was removed
   */
  bool remove(Key key);

  /**
   * @return the number of keys in the hash map
   */
  size_t size() {
    return size_.load();
  };


  /**
   * This class is used to safe guard access to values.
   * Before it is initialized the referenced data_node's access counter would
   * have been incremented.
   */
  class ValueAccessor {
   public:
    friend class HashMap;
    ValueAccessor()
      : access_count_(nullptr)
      , value_(nullptr) {}

    ~ValueAccessor() {
      reset();
    }

    /**
     * @return the address of the value in the data_node.
     */
    Value *value() {
      return value_;
    }

    /**
     * @return whether or not this was initialized.
     */
    bool valid() {
      return (value_ != nullptr && access_count_ != nullptr);
    }

    /**
     * Resets this value accessors, decrementing the access_count and clearing
     * the variables.
     */
    void reset() {
      if (access_count_) {
        access_count_->fetch_add(-1);
        access_count_ = nullptr;
        value_ = nullptr;
      }
    }

   private:
    /**
     * Initializes the value accessor.
     * @param value: the address of the value
     * @param access_count: the address of the value's access_count
     */
    void init(Value * value, std::atomic<int64_t> *access_count) {
      if (access_count_) {  // In case they reuse the object
        reset();
      }

      value_ = value;
      access_count_ = access_count;
    }

    std::atomic<int64_t> *access_count_;
    Value * value_;
  };


  /**
   * @param key: The key
   * @param depth: The depth
   * @return the position this key belongs in at the specified depth.
   */
  uint64_t get_position(Key &key, size_t depth);

  /**
   * @return the maximum depth of the hash map, any depth beyond this would
   * not produce any non-zero positions.
   */
  uint64_t max_depth();

  /**
   * Outputs the positions a key belongs in at each depth.
   * @param key: The Key
   */
  void print_key(Key &key);

 private:
  class Node;
  friend class Node;
  friend class ForceExpandOp;
  typedef std::atomic<Node *> Location;


  /**
   * This class is used to differentiate between data_nodes and array_nodes/
   */
  class Node {
   public:
    Node() {}
    virtual ~Node() {}

    /**
     * @return whether or not this instance is an ArrayNode sub type
     */
    virtual bool is_array() = 0;

    /**
     * @return whether or not this instance is an DataNode sub type
     */
    virtual bool is_data() = 0;
  };

  /**
   * This class is used to hold the secondary array structure
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
     * @param pos: The position to get the address of.
     * @return the address of a position on the internal array
     */
    Location *access(uint64_t pos) {
      assert(pos < len_ && pos >=0);
      return &(internal_array_[pos]);
    }

    /**
     * @return whether or not this instance is an ArrayNode sub type
     */
    bool is_array() {
      return true;
    }

    /**
     * @return whether or not this instance is an DataNode sub type
     */
    bool is_data() {
      return false;
    }

   private:
    uint64_t len_;
    std::unique_ptr<Location[]> internal_array_;
  };

  /**
   * This class is used to hold a key and value pair.
   * It is hazard pointer protected.
   */
  class DataNode : public Node, public tervel::util::memory::hp::Element {
   public:
    explicit DataNode(Key k, Value v)
      : key_(k)
      , value_(v)
      , access_count_(0) {}

    ~DataNode() { }

    /**
     * @return whether or not this instance is an ArrayType sub type
     */
    bool is_array() {
      return false;
    }

    /**
     * @return whether or not this instance is an DataNode sub type
     */
    bool is_data() {
      return true;
    }

    Key key_;
    Value value_;
    std::atomic<int64_t> access_count_;
  };


  /**
   * TODO(steven): add description
   * TODO(steven): move into a file.
   */
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
   * Increases the capacity of the hash map by replacing a data node reference
   * with a reference to an array node containing a reference to that data node
   * @param loc: The location to expand at
   * @param curr_value: The current value  (data node) at the location
   * @param next_position: The position the data node belongs at the next depth.
   */
  void expand_map(Location * loc, Node * curr_value, size_t depth);

  /**
   * This is a wrapper for hazard pointers.
   * If it returns true then the value has been assigned the current value
   * of loc and it is hazard pointer protected.
   * @param  loc   the location to dereference a Node object from
   * @param  value the destination to write the Node objet pointer
   * @return       whether or not it was able to dereference and hazard pointer
   * watch a node object.
   */
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

    assert(tervel::util::memory::hp::HazardPointer::is_watched(   temp) == true);
  }

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


bool hp_check_empty() {
  return !tervel::util::memory::hp::HazardPointer::hasWatch(
      tervel::util::memory::hp::HazardPointer::SlotID::SHORTUSE);
}

template<class Key, class Value, class Functor>
bool HashMap<Key, Value, Functor>::
at(Key key, ValueAccessor &va) {
  assert(hp_check_empty() && " Error: Function Did not release hp watch ");
  Functor functor;
  key = functor.hash(key);

  size_t depth = 0;
  uint64_t position = get_position(key, depth);
  Location *loc = &(primary_array_[position]);
  Node *curr_value;


  bool op_res = false;

  tervel::util::ProgressAssurance::Limit progAssur;
  while (true) {
    if (progAssur.isDelayed()) {
      ForceExpandOp *op = new ForceExpandOp(this, loc, depth);
      util::ProgressAssurance::make_announcement(
            reinterpret_cast<tervel::util::OpRecord *>(op));
      progAssur.reset();
      continue;
    }

    if (!hp_watch_and_get_value(loc, curr_value)) {
      continue;
    } else if (curr_value == nullptr) {
      break;
    } else if (curr_value->is_array()) {
      ArrayNode * array_node = reinterpret_cast<ArrayNode *>(curr_value);
      depth++;
      position = get_position(key, depth);
      loc = array_node->access(position);
      hp_unwatch();
      continue;
    } else {
      assert(curr_value->is_data());
      DataNode * data_node = reinterpret_cast<DataNode *>(curr_value);

      if (functor.key_equals(data_node->key_, key)) {
        int64_t res = data_node->access_count_.fetch_add(1);
        if (res >= 0) {  // its not deleted.
          va.init(&(data_node->value_), &(data_node->access_count_));
          op_res = true;
        } else {
          data_node->access_count_.fetch_add(-1);
          op_res = false;
        }
      }
      hp_unwatch();
      break;
    }
    assert(false);
  }  // while true

  assert(hp_check_empty() && " Error: Function Did not release hp watch ");
  return op_res;
}  // at


template<class Key, class Value, class Functor>
bool HashMap<Key, Value, Functor>::
insert(Key key, Value value) {
  assert(hp_check_empty() && " Error: Function Did not release hp watch ");
  tervel::util::ProgressAssurance::check_for_announcement();

  Functor functor;
  key = functor.hash(key);

  DataNode * new_node = new DataNode(key, value);

  tervel::util::ProgressAssurance::Limit progAssur;
  size_t depth = 0;
  uint64_t position = get_position(key, depth);

  Location *loc = &(primary_array_[position]);
  Node *curr_value;

  bool op_res;
  while (true) {
    if (progAssur.isDelayed()) {
      // TODO(steven): implement an op record.
      // util::ProgressAssurance::make_announcement(
      //       reinterpret_cast<tervel::util::OpRecord *>(op));
      progAssur.reset();
      continue;
    }

    if (!hp_watch_and_get_value(loc, curr_value)) {
      continue;
    } else if (curr_value == nullptr) {
      if (loc->compare_exchange_strong(curr_value, new_node)) {
        size_.fetch_add(1);
        op_res = true;
        break;
      } else {
        continue;
      }
    } else if (curr_value->is_array()) {
      ArrayNode * array_node = reinterpret_cast<ArrayNode *>(curr_value);
      depth++;
      position = get_position(key, depth);
      loc = array_node->access(position);
      hp_unwatch();
      continue;
    } else {  // it is a data node
      assert(curr_value->is_data());
      DataNode * data_node = reinterpret_cast<DataNode *>(curr_value);

      if (data_node->access_count_.load() < 0) {
        if (loc->compare_exchange_strong(curr_value, new_node)) {
          hp_unwatch();
          data_node->safe_delete();
          size_.fetch_add(1);
          op_res = true;
          break;
        } else {
          hp_unwatch();
          continue;
        }
      } else if (functor.key_equals(data_node->key_, key)) {
        op_res = false;
        hp_unwatch();
        break;
      } else {
        // Key differs, needs to expand...
        expand_map(loc, curr_value, depth);
        hp_unwatch();
        continue;
      }   // else key differs
    }  // else it is a data node
    assert(false);
  }  // while true

  if (!op_res) {
    assert(loc->load() != new_node);
    delete new_node;
  }

  assert(hp_check_empty() && " Error: Function Did not release hp watch ");
  return op_res;
}  // insert


template<class Key, class Value, class Functor>
bool HashMap<Key, Value, Functor>::
remove(Key key) {
  assert(hp_check_empty() && " Error: Function Did not release hp watch ");
  Functor functor;
  key = functor.hash(key);

  size_t depth = 0;
  uint64_t position = get_position(key, depth);

  Location *loc = &(primary_array_[position]);
  Node *curr_value;


  tervel::util::ProgressAssurance::Limit progAssur;

  bool op_res = false;
  while (true) {
    if (progAssur.isDelayed()) {
      // TODO(Steven): implement op record
      // util::ProgressAssurance::make_announcement(
      //       reinterpret_cast<tervel::util::OpRecord *>(op));
      progAssur.reset();
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
      hp_unwatch();
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
            hp_unwatch();
            data_node->safe_delete();
            break;
        } else {
          // It is logically deleted, and some other thread will/has already removed and freed it
        }

      }
      hp_unwatch();
      break;

    }  // it is a data node
    assert(false);
  }  // while

  assert(hp_check_empty() && " Error: Function Did not release hp watch ");
  return op_res;
}  // remove


template<class Key, class Value, class Functor>
void  HashMap<Key, Value, Functor>::
expand_map(Location * loc, Node * curr_value, size_t depth) {

  uint64_t next_position = 0;

  ArrayNode * array_node = new ArrayNode(secondary_array_size_);
  if (curr_value != nullptr) {
    assert(curr_value->is_data());
    DataNode *data_node = reinterpret_cast<DataNode *>(curr_value);
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
