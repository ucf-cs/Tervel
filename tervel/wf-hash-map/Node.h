<<<<<<< HEAD
#ifndef TERVEL_WFHM_NODE_H
#define TERVEL_WFHM_NODE_H 

#include <stdio.h>
#include "tervel/util/descriptor.h"

class Node : public util::memory::rc::Descriptor{
public:
=======
//REVIEW(steven) each class should have its own file.
//
#ifndef TERVEL_UTIL_NODE_H //REVIEW(steven) this should be the file path
#define TERVEL_UTIL_NODE_H  // TERVEL_HASHMAP_NODE_H

using namespace std; //REVIEW(steven) dont do this, specify std when needed
template <typename K, typename V>
//REVIEW(steven) no space here
class Node {
private:
	//REVIEW(steven) this class should only havevirtual isPairNode/isArrayNode
	K key;
	V value;
	Node *next;
public:
	Node(const K &key, const V &value) : key(key), value(value), next(NULL) {
	}
	PairNode(const K &key, const V &value) : key(key), value(value), next(NULL) {
	}
	ArrayNode(const K &key, const V &value) : key(key), value(value), next(NULL){
	}
	K getKey() const {
		return key;
	}
	V getValue() const {
		return value;
	}


};

//REVIEW(steven) PairNode should extend Node and util::memory::rc::Descriptor
class PairNode {
private:
	K key;
	V value;
	Node *next;
public:

	bool isPairNode(){
		return true;
	}
	bool isArrayNode(){
		return false;
	}
};

//REVIEW(steven) PairNode should extend Node
class ArrayNode {
private:
	K key;//Not needed by this class, should have an array like the hashmpa
	// the constructor should specify the length of the array.
	V value
	Node *next;
public:

	bool isPairNode(){
		return false;
	}
	bool isArrayNode(){
		return true;
	}
>>>>>>> 944f63fd3331eb142f71d573c2524e2e588bf037

	virtual bool isPair() = 0;
	virtual bool isArray() = 0;
};
<<<<<<< HEAD
=======

//REVIEW(steven) should be in its own file
//should take several templates
//Key, Value, and a comparator,
class HashMap{
private:
	Node **table; //REVIEW(steven) should be std::atomic<Node *> *
public:
	HashMap(){
		table = new Node*[array_length];
		for (int i = 0; i < array_length; i++){
			table[i] = NULL; //REVIEW(steven) use nullptr instead
		}
	}
	int get(int key) {

	}
	void insert(int key, int value){

	}
	bool update(int key, int oldValue, int newValue){

	}
	bool remove(int key, int value){

	}
>>>>>>> 944f63fd3331eb142f71d573c2524e2e588bf037

