#ifndef __wfv_private__
#define __wfv_private__ 1

#include "wfv_classes.hpp"

void WFVector::printVector(){
	long p;
	printf("Vector (Size: %d): [", size());
	for(p=0; p<csize.load(); p++){
		ArrayElement *spot=getSpot(p);
		void * cvalue=(void *)(spot->load());
		printf("[%ld %p], ",p, cvalue);
	}
	printf(" ..... ");
	for(p=p; p<currentCapacity.load(); p++){
		ArrayElement *spot=getSpot(p);
		void * cvalue=(void *)(spot->load());
		if(cvalue != NOT_VALUE)
			printf("[%ld %p], ", p,cvalue);
	}
	
	printf("]\n");
	
}



#ifdef DEF_PB_LOCK
	long WFVector::pushBackwLock(void * value){
		std::lock_guard<std::mutex> lock(tailOpLock);

		int pos=csize;
		csize++;
		ArrayElement *spot=getSpot(pos);
		spot->store(value);
		
		return pos;
	};
	bool WFVector::popBackwLock(void * &value){
		std::lock_guard<std::mutex> lock(tailOpLock);
		
		if(csize>0){
			csize=csize-1;
			ArrayElement *spot=getSpot(csize);
			value=	spot->exchange(NOT_VALUE);
			return true;
		}
		else{
			return false;
		}
		
	};
#endif
	


long WFVector::pushBackwRA(void * value){
	if(!Value::isValid(value) ){
		assert(false);
		return -1;
	}
	
	long pushedPos=csize.load();
	while(true){// TO BOUND
		ArrayElement *spot=getSpot(pushedPos);

		void *expected=	spot->load();
		
		if( expected==NOT_VALUE){
			if(spot->compare_exchange_weak(expected, value)	){
				csize.fetch_add(1);
				return pushedPos;
			}
		}
		pushedPos++;
		continue;
	
	}

}

long WFVector::pushBackOnly(void * value){
	if(!Value::isValid(value) ){
		assert(false);
		return -1;
	}
	
	long pushedPos=csize.fetch_add(1);
	ArrayElement *spot=getSpot(pushedPos);

	spot->store(value,std::memory_order_relaxed);
	return pushedPos;
}


bool WFVector::popBackwRA(void * &value){

	long popPos=csize.load();
	while(true){// TO BOUND
		if(popPos<=0){
			return false;
		}
		
		ArrayElement *spot=getSpot(popPos-1);
		void *current= spot->load();
	
		if(spot->compare_exchange_weak(current, NOT_VALUE)	){			
			csize.fetch_add(-1);
			value=current;
			return true;
		}
		popPos--;
		continue;
	
	}

}

bool WFVector::popBackOnly(void * &value){

	long popPos=csize.fetch_add(-1);
	if(popPos<=0){
		csize.fetch_add(1);
		return false;
	}
	ArrayElement *spot=getSpot(popPos);
	value=spot->load(std::memory_order_relaxed);
	spot->store(NOT_VALUE,std::memory_order_relaxed);

	return true;
}


long WFVector::pushBackwE(void * value){
	
	if(!Value::isValid(value) ){
		assert(false);
		return -1;
	}
	checkOpTable();
		
	PushOp* op=new PushOp(value);

	long pos=op->begin(this);
	op->safeFree();

	csize.fetch_add(1);
	return pos;

}


bool WFVector::popBackwE(void * &value){
	checkOpTable();

	PopOp *op=new PopOp();
	void *temp=op->begin(this);
	op->safeFree();
	if(temp){
		value=temp;
		assert(temp != NULL);
		csize.fetch_add(-1);
		return true;
	}
	else{
		return false;
	}
	
}


bool WFVector::at(int idx, void * &value){
	checkOpTable();
	
	std::atomic<void *> dummy(NULL);
	controlWord=&dummy;
	if(idx < currentCapacity.load(std::memory_order_relaxed)){
		ArrayElement *spot=getSpot(idx);
		
		while(true){// TO BOUND
			if(fcount++ == MAX_FAILURES){
				ReadOp *op = new ReadOp(idx);
				controlWord=&(op->value);
				announceOp(op);

				bool res=op->s_execute(this, value);

				resetMyOp();
				
				op->safeFree();
				return res;
			}
			void *cvalue=spot->load(std::memory_order_relaxed);

			if(Helper::isHelper(cvalue)){
				Helper *tHelper=Helper::unmark(cvalue);
	            if(tHelper->watch(cvalue, spot)){
	            	//Helper::remove(this, idx, cvalue);
	            	cvalue=tHelper->readThrough();
					tHelper->unwatch();
	            }
			}

			if(cvalue== NOT_VALUE){
				return false;
			}
			else{
				assert(Value::isValid(cvalue));
				value=cvalue;
				return true;
			}
		}
	}
	else{
		return false;
	}


};


bool WFVector::cas(int idx, void * &expValue, void *newValue){
	checkOpTable();

	std::atomic<void *> dummy(NULL);
	controlWord=&dummy;
	if(!Value::isValid(expValue) ){
		assert(false);
		return -1;
	}
	if(idx < currentCapacity.load()){
		ArrayElement *spot=getSpot(idx);
		
		while(true){// TODO BOUND
			void *cvalue=spot->load(std::memory_order_relaxed);

			if(Helper::isHelper(cvalue)){
				Helper *tHelper=Helper::unmark(cvalue);
	            if(tHelper->watch(cvalue, spot)){
	            	if(expValue !=tHelper->readThrough()){
	            		expValue=cvalue;
	            		tHelper->unwatch();
						return false;
	            	}
	            	Helper::remove(this, idx, cvalue);
					tHelper->unwatch();
	            }
			}
			else if(cvalue == expValue){
				if(spot->compare_exchange_strong(cvalue, newValue)){
					return true;
				}
				else if(Helper::isHelper(cvalue)){
					continue;
				}
			}
			expValue=cvalue;
			return false;
			
		}
	}
	else{
		return false;
	}
}

bool WFVector::insertAt(int pos, void *value){
	checkOpTable();
	InsertAt* op = new InsertAt(pos, value);
	
	bool res=op->s_execute(this);
	
	if (res) {
		op->cleanup(this,pos);
		csize.fetch_add(1);
	}
	

	op->safeFree();
	return res;

}

bool WFVector::eraseAt(int pos, void * &value){
	checkOpTable();

	EraseAt* op = new EraseAt(pos);
	bool res=op->s_execute(this);
	
	if (res) {
		op->cleanup(this,pos);
		csize.fetch_add(-1);
		value=op->getResult();
		return true;
	}
	op->safeFree();
	//throw out of bounds exception;
	return res;

}

#include "wfv_announce.hpp"

#endif