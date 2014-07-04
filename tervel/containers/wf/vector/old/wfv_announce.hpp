#ifndef __WFV_ANNOUNCE_HPP_
#define __WFV_ANNOUNCE_HPP_ 1

#include "wfv_macros.hpp"
#include "WFVector.hpp"

#define MAX_DELAY 1000000

__thread int tl_helpdelay=0;
__thread int checkID=0;
__thread bool isHelping=false;

void WFVector::checkOpTable(){
	fcount=0;
	controlWord=NULL;
	if(tl_helpdelay++ < MAX_DELAY){
		return;
	}

	
	tl_helpdelay=0;
	checkID=(checkID+1) % vector_nThreads;

	OpRecord *op= (OpRecord *)(opTable[checkID].load());

	if (op==NULL) {
		return;
	}
	isHelping=true;

	if(op->watch(op, (std::atomic<void *> *)&opTable[checkID])){

		if (opTable[checkID].load() == op) {
			op->execute(this);
		}

		op->unwatch();

	}
	isHelping=false;
	return;
}

void WFVector::announceOp(void *op){
	assert(opTable[threadID]== NULL);
	if (!isHelping) {
		opTable[threadID]=(OpRecord *)op;
	}
}
void WFVector::resetMyOp(){
	fcount=0;
	opTable[threadID]=NULL;
}


#endif