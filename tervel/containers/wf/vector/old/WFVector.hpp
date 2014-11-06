#ifndef __WFVECTOR_HPP__
#define  __WFVECTOR_HPP__

#include "wfv_macros.hpp" 

class WFVector{
public:

	int vector_nThreads;
	std::atomic<long> csize;
	ArrayType vectors[MAX_ARRAYS];
	
	int initialCapacity;
	int initialCapPow2;

	std::atomic<long> currentCapacity;
	
	std::atomic<void *> * opTable; 


	WFVector(int capacity, int threads){
		int i;
		for(i=1; i<MAX_ARRAYS; i++)
		{
			vectors[i]=(NULL);
		}
		//TODO round up to nearest power of two!
		if(capacity==0){
			initialCapacity=8;
			initialCapPow2=3;
		}
		else{
			const int leftc=__builtin_clz(capacity);//Returns the number of leading 0-bits in x, starting at the most significant bit position. If x is 0, the result is undefined.
			const int rightc=__builtin_ctz(capacity);//Returns the number of trailing 0-bits in x, starting at the least significant bit position. If x is 0, the result is undefined.
			if(sizeof(int)*8-leftc-rightc == 1){
				//Its a power of two!
				initialCapacity=capacity;
				initialCapPow2=rightc;
			}
			else{//Round up!
				initialCapPow2=sizeof(int)*8-leftc;
				initialCapacity=(0x1)<<initialCapPow2;
				
			}
		}
		
		//printf("Vector IC: %d and ICP2 %d\n",initialCapacity,initialCapPow2);
		vector_nThreads=threads;
		opTable=new std::atomic<void *>[threads]();
		
		ArrayType initilArray= new ArrayElement [initialCapacity]();
		vectors[0]=(initilArray);
		currentCapacity.store(initialCapacity);
		csize.store(0);

	}

	~WFVector(){
		//TODO
	}
	
	bool at(int idx, void * &value);
	bool cas(int idx, void * &expValue, void *newValue);
	
	
	void attachThread(){ 
		threadID=activeThreads.fetch_add(1);
	}
	void dettachThread(){ }
	
	
	long pushBackOnly(void * value);
	long pushBackwRA(void * value);
	long pushBackwE(void * value);
	
	bool popBackwRA(void * &value);
	bool popBackwE(void * &value);
	bool popBackOnly(void * &value);
	
#ifdef DEF_PB_LOCK
	std::mutex tailOpLock;
	
	long pushBackwLock(void * value);
	bool popBackwLock(void * &value);
#endif

	bool insertAt(int pos, void *value);
	bool eraseAt(int pos, void *&value);
	
	int size(){
		return csize.load();
	};
	int capacity(){return currentCapacity.load();};

	void printVector();

	ArrayElement*  getSpot(int rawPos);
	ArrayType expandVector(int pos);

	void checkOpTable();
	void announceOp(void *op);
	void resetMyOp();
};
	
ArrayType WFVector::expandVector(int pos){
	std::atomic<ArrayType>* temp=(std::atomic<ArrayType>*)(&(vectors[pos]));
	ArrayType lvector=temp->load();

	
	if(lvector==NULL){
		int newCap=0x1<<(initialCapPow2+pos);
		
		ArrayType newArray= new ArrayElement[newCap]();
		if(temp->compare_exchange_strong(lvector, newArray)){
			
			currentCapacity.fetch_add(newCap);
			
			//printf("Expanded[%d]: Total Elements: %ld Old Cap: %ld Added Cap: %d Total Cap: %ld \n", pos,csize.load(),oldCap, newCap, currentCapacity.load());
			return newArray;
		}
		else{
			delete [] newArray;
		}
		
	}
	return lvector;
	
}

ArrayElement*  WFVector::getSpot(int rawPos){
	assert(rawPos >=0);
	if (rawPos < initialCapacity) {
		ArrayType lvector1=vectors[0];//.load(std::memory_order_relaxed);
		return &(lvector1[rawPos]);
	}
	else {
		
		static const int nobits = (sizeof(unsigned int) << 3) - 1;
		size_t pos=rawPos+initialCapacity;
		size_t num = nobits - __builtin_clz(pos);
	
		size_t arrayPos1 = pos ^ (1 << num);
		size_t array1=num-initialCapPow2;
	
		ArrayType lvector1=vectors[array1];//.load(std::memory_order_relaxed);
	
		if (lvector1 == NULL) {
			//Expand and set lvector to current value
			lvector1=expandVector(array1);
			assert(lvector1 != NULL);
		};
	
		return &(lvector1[arrayPos1]);
	}//End Else Not first array
	
}


#include "wfv_private.hpp"

#endif
