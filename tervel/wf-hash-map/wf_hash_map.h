#ifndef TERVEL_WFHM_HASHMAP_H
#define TERVEL_WFHM_HASHMAP_H 

#include "tervel/wf-hash-map/node.h"
#include "tervel/wf-hash-map/pair_node.h"
#include "tervel/wf-hash-map/array_node.h"

#include <stdlib.h>

#include <atomic>

const std::atomic<Node *> array[64];
const int maxFailCount = 100000;
//need to implement thread-local variable threadID as well as global array watchedNodes.
class HashMap{
 public:
  HashMap(){
     table = new Node*[arrayLength];
       for (int i = 0; i < arrayLength; i++){
         table[i] = nullptr;
       }
   }
   HashMap(){

   }
  //head and arrayPow is not implemented yet. neither is markDataNode() not sure even which file this function belongs in. 
  int get(T Key) {
    //not 100% sure if hashKey(key) means to do Key.hash() or not.
    uint64_t hash = currHash = Key.hash();
    std::atomic<Node *> local = head;
    int result = null;
    for (int r = 0; r < keySize - arrayPow; r += arrayPow){
      int pos = hash&(arrayLength - 1);
      hash = hash << arrayPow;
      node = getNode(local, pos);
      if (isArrayNode(node))
      local = node;
      else if (node == null)
        break;
      else {
        watch(node);
        if (node != getNode(local, pos) {
          int failCount = 0;
          while (node != getNode(local, pos)) {
            node = getNode(local, pos);
            watch(node);
            failCount++;
            if (failCount > maxFailCount) {
              markDataNode(local, pos);
              local = expandMap(local, pos, r);
              break;
            }
          }
          if (isArrayNode(node) {
            local = node;
            continue;
          }
          else if (isMarked(node)) {
            local = expandMap(local, pos, r);
            continue;
          }
          else if (node == null) {
            break;
          }
        }
        if(node -> hash == currHash)
          result = node -> value;
        break;
      }
    }
    if (r >= keySize - arrayPow) {
      pos = hash&(arrayLength-1);
      result = local[pos];
    }
    watch(null);
    return result;
  }
  //CAS and markDataNode, isMarked, free not implemented yet.
  bool insert(T Key,int value){
    uint64_t hash = Key.hash();
    std::atomic<Node *> insertThis = allocateNode(value,hash);
    std::atomic<Node *> local = head;
    for (int r=0; r < keySize - arrayPow; r+=arrayPow) {
      //pos is an int or is it uin64_t?
      int pos = hash&(arrayLength - 1);
      uint64_t hash = hash >> arrayPow;
      int failCount = 0;
      std::atomic<Node *> node = getNode(local, pos);
      while (true) {
        if (failCount > maxFailCount) {
          node = markDataNode(local, pos);
        if (node == null)
          if ((node = CAS(local[pos], null, insertThis)) == null) {
            watch(null);
            return true;
          }
        if (isMarked(node)) 
          node = expandMap(local, pos, r);
        if (isArrayNode(node)) {
          local = node;
          break;
        else {
          watch(node);
          std::atomic<Node *> node2 = getNode(local, pos);
          if (node != node2) {
            failCount++;
            node = node2;
            continue;
          }
          else if (node -> hash == insertThis -> hash) {
            watch(null);
            free(insertThis);
            return false;
          }
          else {
            node = expandMap(local, pos, r);
            if(isArrayNode(node)) {
              local = node;
              break;
            else {
              failCount++;
            }
          }
        }
      }
    }
    free(insertThis);
    watch(null);
    pos = hash&(arrayLength - 1);
    int currValue = local[pos];
    if (currValue == null) 
      return (CAS(local[pos], null, value) == null);
    else
      return false;
  }
  bool update(T Key,int oldValue,int newValue){

  }
  bool remove(T Key,int value){

  }
  std::atomic<Node *> expandMap ( std::atomic<Node *> local, int pos, int right){
     
  }
  //unsure what datatype value should be
  void watch(int value) {
    watchedNodes[threadID] = value;
  }
  
  std::atomic<Node *> allocateNode(int value, uint64_t hash) {
    //WIP
    return node;
  }
  void safeFreeNode(std::atomic<Node *> nodeToFree) {
  //WIP
  }
  
 private:
  int arrayLength_
  int arrayPow_
};