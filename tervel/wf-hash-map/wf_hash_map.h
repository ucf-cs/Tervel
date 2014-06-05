#ifndef TERVEL_WFHM_HASHMAP_H
#define TERVEL_WFHM_HASHMAP_H 

#include "tervel/wf-hash-map/node.h"
#include "tervel/wf-hash-map/pair_node.h"
#include "tervel/wf-hash-map/array_node.h"

#include <stdlib.h>

#include <atomic>

const std::atomic<Node *> array[64];

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
	int get(int key) {

	}
	void insert(int key, int value){

	}
	bool update(int key, int oldValue, int newValue){

	}
	bool remove(int key, int value){

	}
private:
	int array_length;
};