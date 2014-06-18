#ifndef TERVEL_WFHM_ARRAY_H
#define TERVEL_WFHM_ARRAY_H

#include "tervel/wf-hash-map/node.h"  // full path
#include "tervel/wf-hash-map/wf_hash_map.h"

template <class T>
class Array : public Node {
 public:
  explicit Array(const size_t size)
    : array_(new std::atomic<Node *>[size]() )
    , length_(size) {}

  Node GetNode(size_t pos) {  // use size_t not int.
    return array_[pos];
  }
  bool IsPairNode() { 
    return false;
  }
  bool IsArrayNode() { 
    return true; 
  }


 private:
  std::unique_ptr< std::atomic<Node *> >array_;
  size_t length_;
};
