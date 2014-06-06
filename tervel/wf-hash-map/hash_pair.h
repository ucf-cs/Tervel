#ifndef TERVEL_WFHM_HASHPAIR_H
#define TERVEL_WFHM_HASHPAIR_H

#include "tervel/wf-hash-map/node.h"  // full path
#include "tervel/util/descriptor.h"

template <typename V, typename T, typename hash, typename compare>
class HashPair : public Node, public util::memory::rc::Descriptor {
  // http://www.cprogramming.com/tutorial/multiple_inheritance.html
 public:
  HashPair(V value, T key)
    : value_(value)
    , key_(key) {};

  V& value() {
    return value_;
  }
  T& key() {
    return key_.key();
  }

  bool isPairNode() { return true; }
  bool isArrayNode() { return false; }

 private:  // added under scores
  Key<T, hash, compare> key_;
  V value_;
  Node *next_;  // Whats this for?
};
