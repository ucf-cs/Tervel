#ifndef TERVEL_WFHM_HASHMAP_H
#define TERVEL_WFHM_HASHMAP_H 

#include "tervel/wf-hash-map/node.h"
#include "tervel/wf-hash-map/pair_node.h"
#include "tervel/wf-hash-map/array_node.h"

#include <stdlib.h>

#include <atomic>

const std::atomic<Node *> array[64];
const int MAX_FAIL_COUNT = 100000;
//need to implement thread-local variable threadID as well as global array watchedNodes.
class HashMap{
 public:
  HashMap(){
     table = new Node*[array_length];
       for (int i = 0; i < array_length; i++){
         table[i] = nullptr;
       }
   }
   HashMap(){

   }
  //head and arrayPow is not implemented yet. neither is markDataNode() not sure even which file this function belongs in. 
  int Get(T key) {
    //not 100% sure if hashKey(key) means to do Key.hash() or not.
    uint64_t hash = curr_hash = key.Hash();
    std::atomic<Node *> local = head;
    int result = null;
    for (int r = 0; r < key_size - array_pow; r += array_pow){
      uint64_t pos = hash&(array_length - 1);
      hash = hash << array_pow;
      node = GetNode(local, pos);
      if (node.IsArrayNode())
      local = node;
      else if (node == null)
        break;
      else {
        Watch(node);
        if (node != GetNode(local, pos) {
          int fail_count = 0;
          while (node != GetNode(local, pos)) {
            node = GetNode(local, pos);
            Watch(node);
            fail_count++;
            if (fail_count > MAX_FAIL_COUNT) {
              MarkDataNode(local, pos);
              local = ExpandMap(local, pos, r);
              break;
            }
          }
          if (node.IsArrayNode()) {
            local = node;
            continue;
          }
          else if (IsMarked(node)) {
            local = ExpandMap(local, pos, r);
            continue;
          }
          else if (node == null) {
            break;
          }
        }
        if(node->hash == curr_hash)
          result = node->value;
        break;
      }
    }
    if (r >= key_size - array_pow) {
      pos = hash&(array_length-1);
      result = local[pos];
    }
    Watch(null);
    return result;
  }
  //CAS and markDataNode, isMarked, free not implemented yet.
  bool Insert(T key,int value){
    uint64_t hash = key.Hash();
    std::atomic<Node *> insert_this = AllocateNode(value,hash);
    std::atomic<Node *> local = head;
    for (int r=0; r < key_size - array_pow; r+=array_pow) {
      //pos is an int or is it uin64_t?
      int pos = hash&(array_length - 1);
      uint64_t hash = hash >> array_pow;
      int fail_count = 0;
      std::atomic<Node *> node = GetNode(local, pos);
      while (true) {
        if (fail_count > MAX_FAIL_COUNT) {
          node = MarkDataNode(local, pos);
        if (node == null)
          if ((node = CAS(local[pos], null, insert_this)) == null) {
            Watch(null);
            return true;
          }
        //IsMarked is TODO
        if (IsMarked(node)) 
          node = ExpandMap(local, pos, r);
        if (Node.IsArrayNode()) {
          local = node;
          break;
        else {
          Watch(node);
          std::atomic<Node *> node2 = GetNode(local, pos);
          if (node != node2) {
            fail_count++;
            node = node2;
            continue;
          }
          else if (node->hash == insert_this->hash) {
            Watch(null);
            Free(insert_this);
            return false;
          }
          else {
            node = ExpandMap(local, pos, r);
            if(node.IsArrayNode()) {
              local = node;
              break;
            else {
              fail_count++;
            }
          }
        }
      }
    }
    Free(insert_this);
    Watch(null);
    pos = hash&(array_length - 1);
    int curr_value = local[pos];
    if (curr_value == null) 
      return (CAS(local[pos], null, value) == null);
    else
      return false;
  }
  bool Update(T key,int old_value,int new_value){
    uint64_t hash = key.Hash();
    std::atomic<Node *> insert_this = AllocateNode(new_value, hash);
    std::atomic<Node *> local = head;
    bool result = false;
    for(int r = 0; r < key_size - array_pow; r += array_pow) {
      uint64_t pos = hash&(array_length -1); 
      hash = hash>>array_pow;
      node = GetNode(local, pos);
      if(IsArrayNode(node)) 
        local = node;
      else if(IsMarked(node))
        local = ExpandMap(local, pos, r);
      else if(node == null) 
        break;
      else {
        Watch(node);
        if(node != GetNode(local, pos)) {
          fail_count = 0;
          while(node != GetNode(local, pos)) {
            node = GetNode(local, pos);
            Watch(node);
            fail_count++;
            if(fail_count > MAX_FAIL_COUNT) {
              MarkDataNode(local, pos);
              local = ExpandMap(local, pos, r);
              break;
            }
          }
          if(IsArrayNode(node)) {
            local = node;
            continue;
          }
          else if(IsMarked(node)) {
            local = ExpandMap(local, pos, r);
            continue;
          }
          else if(node == null) break;
        }
        if(node->hash == insert_this->hash) {
          if(node->value != old_value) {
            break;
          }
          if((std::atomic<Node *> node2 = CAS(local[pos], node, insert_this)) == node) {
            result = true;
            break;
          }
          else{
            if(IsArrayNode(node2))
              local = node2;
            //TODO implement Unmark()
            else if(IsMarked(node2)^Unmark(node2) == node) 
              local = ExpandMap(local, pos, r);
            else
              break;
          }
        }
        else
          break;
      }
    }
    if(r >= key_size - array_pow) {
      Free(insert_this);
      pos = hash&(array_length - 1);
      curr_value = local[pos];
      if(curr_value == old_value) {
        result = (CAS(local[pos], old_value, new_value) == old_value);
      }
      else
        result = false;
    }
    else if(result) 
      SafeFreeNode(node);
    else if(node != null) 
      Free(insert_this);
    Watch(null);
    return result;
  }
  bool Remove(T key,int value){
    uint64_t curr_hash = key.Hash();
    uint64_t hash = key.Hash();
    std::atomic<Node *> local = head;
    bool result = false;
    for(int r = 0; r < key_size - array_pow; r += array_pow) {
      uint64_t pos = hash&(array_length - 1);
      hash = hash>>array_pow;
      node = GetNode(local, pos);
      if(IsArrayNode(node))
        local = node;
      else if(IsMarked(node)) 
        local = ExpandMap(local, pos, r);
      else if(node == null) 
        break;
      else {
        Watch(node);
        if(node != GetNode(local, pos)) {
          fail_count = 0;
          while(node != GetNode(local, pos)) {
            node = GetNode(local, pos);
            Watch(node);
            fail_count++;
            if(fail_count > MAX_FAIL_COUNT) {
              MarkDataNode(local, pos);
              node = ExpandMap(local, pos, r);
              break;
            }
          }
          if(IsArrayNode(node)) {
            local = node;
            continue;
          }
          else if(IsMarked(node)) {
            local = ExpandMap(local, pos, r);
            continue;
          }
          else if(node == null)
            break;
        }
        if(node->hash == curr_hash) {
          if(node->value != value)
            break;  
          if((std::atomic<Node *> node2 = CAS(local[pos], node, null)) == node) {
            SafeFreeNode(node);
            result = true;
            break;
          }
          else {
            if(IsArrayNode(node2))
              local = node2;
            else if(IsMarked(node2)^Unmark(node2) == node)
              local = ExpandMap(local, pos, r);
            else
              break;
          }
        }
        else 
          break;
      }
    }
    if(r >= key_size - array_pow) {
      Free(insert_this);
      pos = hash&(array_length-1);
      curr_value = local[pos];
      if(curr_value == value)
        result = (CAS(local[pos], value, null) == value);
      else
        result = false;
    }
    Watch(null);
    return result;
  }
  std::atomic<Node *> ExpandMap ( std::atomic<Node *> local, uint64_t pos, int right){
    std::atomic<Node *> node = GetNode(local, pos);
    Watch(node);
    if(IsArrayNode(node)
      return node;
    std::atomic<Node *> node2 = GetNode(local, pos);
    if(node != node2) 
      return node2;
    //TODO implement array_node
    std::atomic<Node *> anode = alloc(sizeof(array_node));
    uint64_t new_pos = (node->hash>>(right+array_pow))&(array_length -1);
    anode[new_pos] = node;
    if((node2 = CAS(local[pos], node, anode)) == node)
      return anode;
    else {
      anode[new_pos] = null;
      Free(anode);
      return node2;
    }
  }
  //unsure what datatype value should be
  void Watch(int value) {
    watchedNodes[thread_id] = value;
  }
  
  std::atomic<Node *> AllocateNode(int value, uint64_t hash) {
    //TODO
    return node;
  }
  void SafeFreeNode(std::atomic<Node *> node_to_free) {
  //TODO
  }
  std::atomic<Node *> AllocateNode(int value, uint64_t hash) {
  //TODO
  }
  std::atomic<Node *> GetNode(std::atomic<Node *> local, uint64_t pos) {
    std::atomic<Node *> res = &local[pos];
    return res;
  }
  std::atomic<Node *> IsMarked(std::atomic<Node *> node) {
    std::atomic<Node *> res = (node&0x1);
    return res;
  }
  std::atomic<Node *> MarkDataNode(std::atomic<Node *> local, uint64_t pos) {
    //TODO
  }
  std::atomic<Node *> Unmark(std::atomic<Node *> node){
    std::atomic<Node *> res = (node | 0x1);
    return res;
  }
 private:
  int array_length_
  int array_pow_
  std::atomic<Node *> head_
};