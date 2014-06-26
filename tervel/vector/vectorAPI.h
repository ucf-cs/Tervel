#ifndef __VECTORAPI__
#define __VECTORAPI__
#include "WFVector.hpp" 

WFVector *vec;

void p_initVector(int size, int nThreads){
	vec=new WFVector(size, nThreads);

};

void p_destroyVector(){
	
	delete vec;
	
};


void p_attachThread(){
	vec->attachThread();
};


void p_dettachThread(){
	vec->dettachThread();
};

void * p_readVector(int pos){
	void *temp=NULL;
	vec->at(pos, temp);
	return temp;
};


bool p_casVector(int pos, void * expected, void *vcount){
	void *temp=expected;
	return vec->cas(pos, temp, vcount);
};


int p_pushBack(void *v){
#ifdef DEF_PB_ONLY
#define NO_ERASE 1
#define NO_INSERT 1
#define NO_POP 1
  return vec->pushBackOnly(v);
#elif defined DEF_PB_RA
#define NO_ERASE 1
#define NO_INSERT 1
#define NO_POP 1
 
  return vec->pushBackwRA(v);
#elif defined DEF_PB_E
	return vec->pushBackwE(v);
#elif defined DEF_PB_LOCK
#define NO_ERASE 1
#define NO_INSERT 1
 return vec->pushBackwLock(v);
#else
	#error need to define pushback model
#endif
};

void * p_popBack(){
	void *temp=NULL;
	#ifdef DEF_PB_ONLY
		vec->popBackOnly(temp);
	#elif defined DEF_PB_RA
		vec->popBackwRA(temp);
	#elif defined DEF_PB_E
		vec->popBackwE(temp);
	#elif defined DEF_PB_LOCK
		vec->popBackwLock(temp);
	#else
		#error need to define popback model
	#endif
		
	return temp;
};

bool p_insertAt(int pos, void *v){
	return  vec->insertAt(pos, v);
};

bool p_eraseAt(int pos){
	void *temp=NULL;
	return  vec->eraseAt(pos, temp);
};

bool p_eraseAt(int pos, void * &v){
	return  vec->eraseAt(pos, v);

};

int p_getSize(){
	return vec->size();
};

void p_printVector(){
	vec->printVector();
}
#endif
