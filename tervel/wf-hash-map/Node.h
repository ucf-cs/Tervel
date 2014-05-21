#ifndef TERVEL_UTIL_NODE_H
#define TERVEL_UTIL_NODE_H

using namespace std;
template <typename K, typename V>

class Node {
private:
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

class ArrayNode {
private:
	K key;
	V value
	Node *next;
public:

	bool isPairNode(){
		return false;
	}
	bool isArrayNode(){
		return true;
	}

};
class HashMap{
private:
	Node **table;
public:
	HashMap(){
		table = new Node*[array_length];
		for (int i = 0; i < array_length; i++){
			table[i] = NULL;
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

};