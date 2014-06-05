#ifndef TERVEL_WFHM_NODE_H
#define TERVEL_WFHM_NODE_H 

#include <stdio.h>
#include "tervel/util/descriptor.h"

class Node : public util::memory::rc::Descriptor{
public:

	virtual bool isPair() = 0;
	virtual bool isArray() = 0;
};

