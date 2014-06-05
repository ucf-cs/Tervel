#ifndef TERVEL_WFHM_ARRAY_H
#define TERVEL_WFHM_ARRAY_H 

#include "node.h"
#include "wf_hash_map.h"

template <class T>
class Array : public Node{
public:

	Node get_node(int pos) {
		return array[pos];
	}
	bool isPair() { return false; }
	bool isArray() { return true; }


private:
	V value
	Node *next;

};