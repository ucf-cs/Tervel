#ifndef TERVEL_WFHM_HASHPAIR_H
#define TERVEL_WFHM_HASHPAIR_H 

#include "node.h"

template <class V, class T>
class HashPair : public Node {
public:
	V get_value() {
		return value;
	}
	Key<T> get_key() {
		return key;
	}

	bool isPairNode() { return true; }
	bool isArrayNode() { return false; }

private:
	Key<T> key;
	V value;
	Node *next;
};