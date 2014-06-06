#ifndef TERVEL_WFHM_KEY_H
#define TERVEL_WFHM_KEY_H

// Use two spaces not tabls
template <typename T, typename hash, typename compare>
class Key {
 public:  // one space before public/private
  explicit Key<T>(T key)
    : key_(key)
    , hash_(hash(key)) {}

  ~Key<T>() {}

  T& key() {  // one space before {
    return key_;
  };

  uint64_t& hash() {
    return hash_;
  };

  void index(depth) {
    // Should take the hash map object and based on its initial capacity
    // and expansion rate (size of each allocated array)
    // return the position it belongs at the specified depth
    // this is done by using bitwise operations to divide the binary
    // representation of the hash value of the key into chunks
    // where the length of the first chunck is equal to the 2^X=initial capacity
    // and each subsequenet chunck is equal to 2^Y=sizeof the sub arrays
  };

  bool operator==(const Key &other) const {  // only need class name when it is defined outside of the class
    // return (*this == other); // this is ciruclar logic.
    return compare(key_, other.key);
  }
  bool operator!=(const Key &other) const {
    return !(*this == other);
  }

 private:
  T key_;
  uint64_t hash_;
};
