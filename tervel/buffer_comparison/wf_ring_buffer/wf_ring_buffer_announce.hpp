#ifndef __WF_ANNOUNCE_HPP_
#define __WF_ANNOUNCE_HPP_ 1

#include "wf_macros.hpp"
#include "wf_ring_buffer.hpp"

#define MAX_DELAY 1000000

__thread int tl_helpdelay=0;
__thread int checkID=0;
__thread bool isHelping=false;

void WFRingBuffer::checkOpTable(){
	fcount=0;
	if(tl_helpdelay++ < MAX_DELAY){
		return;
	}

	
	tl_helpdelay=0;
	checkID=(checkID+1) % num_threads_;

	OpRecord *op= (OpRecord *)(opTable[checkID].load());

	if (op==NULL) {
		return;
	}

	//if(op->watch(op, (std::atomic<void *> *)&opTable[checkID])){

    if (opTable[checkID].load() == op) {
        op->execute(this);
    }

	//	op->unwatch();

	//}
	return;
}

void WFRingBuffer::announceOp(void *op){

    if (opTable[threadID].load() != nullptr) {
        OpRecord *op = (OpRecord*) opTable[threadID].load();
        std::cout << op->getResult();
    }

	assert(opTable[threadID].load() == nullptr);
	opTable[threadID].store((OpRecord *)op);
	((OpRecord*)op)->execute(this);
//	opTable[threadID].store((void *) 0x0);
	opTable[threadID].store(nullptr);
	assert(opTable[threadID].load() == nullptr);
	fcount=0;
}


#endif
