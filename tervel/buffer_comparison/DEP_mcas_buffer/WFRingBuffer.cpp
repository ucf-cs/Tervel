//
//  WFRingBuffer.cpp
//  WF RingBuffer
//
//  Created by Steven Feldman on 10/29/13.
//  Copyright (c) 2013 Steven FELDMAN. All rights reserved.
//
//#define DONT_FREE 1

#define HELP_DELAY 1000
#define MAX_FAILURES 1000

#include "WFRingBuffer.h"


//Active Thread, Thread Identifiers
static int nThreads;
std::atomic<int> activeThreads;
__thread int threadID;


//Shared Memory Pools
static std::atomic<RingBuffer::Helper*>	*gl_Helper;

static std::atomic<RingBuffer::DWCAS*>	*gl_DWCAS;

static std::atomic<RingBuffer::DelayedOp*>	*gl_DelayedOp;

//Thread Local Memory Pools
__thread RingBuffer::DelayedOp* tl_DelayedOp;
__thread int		tl_DelayedOp_count;

__thread RingBuffer::Helper*	tl_Helper;
__thread int		tl_Helper_count;

__thread RingBuffer::DWCAS*	tl_DWCAS;
__thread int		tl_DWCAS_count;




//Wait-Free Functions
static __thread int helpDelay;
static __thread int checkID;
static std::atomic<long> *delayedOps;



void RingBuffer::announceDelayedEnqueue(RingBuffer::DelayedOp *dop){
	delayedOps[threadID-1].store((long)dop|(0x1));
}
void RingBuffer::announceDelayedDequeue(RingBuffer::DelayedOp *dop){
	delayedOps[threadID-1].store((long)dop|(0x0));
}
void RingBuffer::unannounceDelay(){
	delayedOps[threadID-1]=0L;
}
void RingBuffer::helpDelayed(){
	if(threadID==0){
		threadID=activeThreads.fetch_add(1)+1;
	}

	if(helpDelay++ != HELP_DELAY){
		return;
	}
	helpDelay=0;

	checkID=(checkID+1)%nThreads;

	long t_op=delayedOps[checkID].load();

	if(t_op== 0L){
		return;
	}
	RingBuffer::DelayedOp *d_op=(RingBuffer::DelayedOp *)(t_op&(~ 0x1));
	bool watched= d_op->watch(&delayedOps[checkID]);

	if(watched){
		if( (t_op&(0x1)) == 0x1 ){
			d_op->rb->enqueue(d_op->value, d_op);
		}
		else{
			void *dummy=NULL;
			d_op->rb->dequeue(dummy, d_op);
		}
		d_op->unwatch();
	}

	return;


}



/**
 This function is used to return an object to its specfic pool.
 If the pool contains more than nThread+1 object, than object count - nThread-1 objects are given to the global pool
 Otherwise the object is put in the local pool.

 **/
template<class T>
void returnToMemoryPool(T *node, int pThreadID, T* &p_lpool, int &p_lpool_count, std::atomic<T*> * &p_glpool){

//#ifdef DEBUG /*used to variafiy that each thread has an accurate count of the objects in its pool */
//	{int check=0;
//		T *tempCheck=p_lpool;
//		while(tempCheck != NULL){
//			check++;
//			tempCheck=tempCheck->next;
//		}int lhc=p_lpool_count; T *lh=p_lpool;
//		assert(check== p_lpool_count);}
//#endif

	if(p_lpool_count >= (nThreads+2) ){//If thread pool has too many elements put it in the global pool

		//First get any objects I am currently giving away.
		T *currentExtra=p_glpool[pThreadID+1].load();
		if(currentExtra != NULL){
			p_glpool[pThreadID+1].compare_exchange_strong(currentExtra, NULL);//TODO: optimize: replace with SWAP
			assert(p_glpool[pThreadID+1].load()== NULL);
		}

		T *currentList=p_lpool;
		node->next=currentList;

		//Than prepend append objects to that list
		int count=p_lpool_count;
		for(count=count; count> (nThreads+2); count--){
			currentList=currentList->next;
		}count--;
		p_lpool_count=count;

		assert(currentList != NULL);
		p_lpool=currentList->next;

		currentList->next=currentExtra;

		//Put The list in the announcement
		T *temp3=p_glpool[pThreadID+1].load(); assert(temp3==NULL);
		bool success=p_glpool[pThreadID+1].compare_exchange_strong(temp3, node); ////TODO: optimize: Can be replaced by a Store, since if it is NULL than no one will set it to a value.
		assert(success);

	}
	else{//Else put it in the thread local pool
		node->next=p_lpool;
		p_lpool=node;
		p_lpool_count++;

	}//End Else put it thread pool


//#ifdef DEBUG /*used to variafiy that each thread has an accurate count of the objects in its pool */
//	{int check=0;
//		T *tempCheck=p_lpool;
//		while(tempCheck != NULL){
//			check++;
//			tempCheck=tempCheck->next;
//		}int lhc=p_lpool_count; T *lh=p_lpool;
//		assert(check== p_lpool_count);}
//#endif

}


/**
	This function retrieves an object from either the thread local pool or global pool. If not suitable object is avaivable, NULL is returned.
 **/
template<class T>
static T * getFromMemoryPool(int pThreadID, T* &p_lpool, int &p_lpool_count, std::atomic<T*> * &p_glpool){

	//#ifdef DEBUG /*used to variafiy that each thread has an accurate count of the objects in its pool */
	//					{int check=0;
	//					T *tempCheck=p_lpool;
	//					while(tempCheck != NULL){
	//						check++;
	//						tempCheck=tempCheck->next;
	//					}int lhc=p_lpool_count; T *lh=p_lpool;
	//						assert(check== p_lpool_count);}
	//#endif


	T *res=p_lpool;

	if(res  != NULL){//If thread Local pool has elements

		if(res->isUnWatched()){//Check if the first element is free
			p_lpool=res->next;//If so update the pointer
		}
		else{//Otherwise iterate through the linked list
			T *temp=res;
			res=temp->next;
			while(res != NULL){

				if(res->isUnWatched()){
					//Some middle element was found
					temp->next=res->next;///Remove it
					assert(temp != res->next);
					assert(temp != res);
					break;//Exit the loop
				}
				else{//otherwise load the next element
					temp=res;
					res=res->next;

				}
			}//End While loop of elements

		}//End Else check the list
	}//End if the list has elements


	if(res != NULL){//If an unwatched element was found
		assert(p_lpool != res);

		p_lpool_count--; //Decrement object count
		res->next=NULL; //Clear Null pointer
		return res; //Return it.
	}
	else{//Else Check the shared pool
		int i;
		for(i=0; i<nThreads; i++){ //Iterate each threads
			T *otherListHead=p_glpool[i].load();
			if(otherListHead != NULL){//If this thread has extrea
				bool success=p_glpool[i].compare_exchange_strong(otherListHead, NULL); //Try to take
				if(success){//If I took


					T *otherList=otherListHead;
					while(otherList != NULL){//Iterate through what I took
						T *otherNext=otherList->next;

						if(res == NULL && otherList->isUnWatched()){//If I have not found an object and this one is unwatched
							res=otherList;//Take this one
						}
						else{//Otherwise put it in my list for later
							otherList->next=p_lpool;
							p_lpool=otherList;
							p_lpool_count++;
						}
						otherList=otherNext;
					}//End While loop over taken elements

//#ifdef DEBUG /*used to variafiy that each thread has an accurate count of the objects in its pool */
//					{int check=0;
//						T *tempCheck=p_lpool;
//						while(tempCheck != NULL){
//							check++;
//							tempCheck=tempCheck->next;
//						}int lhc=p_lpool_count; T *lh=p_lpool;
//						assert(check== p_lpool_count);}
//#endif

					if(res != NULL){//If I found an element
						res->next=NULL;
						return res;//Return it
					}

				}//IF I took it
			}//IF Other Thread Has Free Stuff
		}//End Loop On other threads Stuff
		
		
		return NULL;//no suitable element found
		
	}
}//End get from Pool


/**
 Used to initate static data structutes. 
 Object pools can be shared between Ring Buffer instantous, as well as announucment schemes.
 
 **/

void Init_RingBuffer_Memory_Management(int _nthreads){
	nThreads=_nthreads;
	gl_Helper		= new std::atomic<RingBuffer::Helper*>	[_nthreads] ();

	gl_DWCAS		= new std::atomic<RingBuffer::DWCAS*>	[_nthreads] ();

	gl_DelayedOp = new std::atomic<RingBuffer::DelayedOp*>	[_nthreads] ();


	delayedOps	= new std::atomic<long>					[_nthreads] ();
}

static void Destory_RingBuffer_Memory_Management(){

	int i=0;
	int aThreads=activeThreads.load();
	for(i=0; i< aThreads; i++){
		RingBuffer::DelayedOp *temp2=NULL;
		RingBuffer::DelayedOp *temp= gl_DelayedOp[i].load();
		while(temp != NULL){
			temp2=temp->next;
			delete temp;
			temp=temp2;
		}
	}//End Delete Delayed Ops

	for(i=0; i< aThreads; i++){
		RingBuffer::DWCAS *temp2=NULL;
		RingBuffer::DWCAS *temp=gl_DWCAS[i].load();
		while(temp != NULL){
			temp2=temp->next;
			delete temp;
			temp=temp2;
		}
	}//End Delete DWCAS Ops

	for(i=0; i< aThreads; i++){
		RingBuffer::Helper *temp2=NULL;
		RingBuffer::Helper *temp=gl_Helper[i].load();
		while(temp != NULL){
			temp2=temp->next;
			delete temp;
			temp=temp2;
		}
	}//End Delete DWCAS Ops


	delete gl_DWCAS;
	delete gl_Helper;
	delete gl_DelayedOp;
	delete delayedOps;
}


/**
 Used to assign a unique thread identifier to a thread
 **/
static void INIT_THREAD_ID(){
	if(threadID==0){
		threadID=activeThreads.fetch_add(1)+1;
		tl_DelayedOp_count=0;
		tl_DWCAS_count=0;
		tl_Helper_count=0;
		tl_DelayedOp=NULL;
		tl_DWCAS=NULL;
		tl_Helper=0;
	}

}
static void DESTORY_THREAD_ID(){
	if(threadID==0){
		return;
	}

	{
		RingBuffer::DWCAS *temp=tl_DWCAS;
		if(temp != NULL){
			RingBuffer::DWCAS *temp2=temp;
			while (temp2->next != NULL) {
				temp2=temp2->next;
			}
			RingBuffer::DWCAS * extra= gl_DWCAS[threadID-1].load();
			if(extra != NULL)
				gl_DWCAS[threadID-1].compare_exchange_strong(extra, NULL);
			temp2->next=extra;
			gl_DWCAS[threadID-1].store(temp);
			tl_DWCAS=NULL;
			tl_DWCAS_count=0;
		}
	}


	{
		RingBuffer::Helper *temp=tl_Helper;
		if(temp != NULL){
			RingBuffer::Helper *temp2=temp;
			while (temp2->next != NULL) {
				temp2=temp2->next;
			}
			RingBuffer::Helper * extra= gl_Helper[threadID-1].load();
			if(extra != NULL)
				gl_Helper[threadID-1].compare_exchange_strong(extra, NULL);
			temp2->next=extra;
			gl_Helper[threadID-1].store(temp);
			tl_Helper=NULL;
			tl_Helper_count=0;
		}
	}



	{
		gl_DelayedOp[threadID-1].store(tl_DelayedOp);
		tl_DelayedOp=NULL;
		tl_DelayedOp_count=0;
	}


}


/**
 This is the RingBuffer Constructor: It takes a size parameter
 optimize: force power of two capacity
 **/

RingBuffer::RingBuffer (size_t s ) {

	if(s<2)//Forces minum size
		s=3;
	capacity = s;

	head.store(0);//Sets the initial Head Position
	tail.store(1);//Sets the initial Tail Position

	buff=new std::atomic<void *>[capacity](); //Allocates an array

	int i=0;//Sets the default values in the array
	buff[i++].store(headValue);
	buff[i++].store(tailValue);
	for(i=i; i<capacity; i++){
		buff[i]=notValue;
	}
}



/**
 Todo: add active thread check? Maybe RC the datastructure?

 
 **/
RingBuffer::~RingBuffer () {
	delete  buff;
}

/**
 This function takes a clean-unbitmarked pointer and enqueues into the queue.
 It returns false in the event the queue is full and true otherwie
 
 This function takes a clean-unbitmarked pointer and possibly a DelayedOp Pointer
 
 If dop is NOT NULL, than this thread is helping to complete an enqueue on behalf of another thread who is unable to do so.


 **/
bool RingBuffer::enqueue(void *value, DelayedOp *dop){

	//First get a best guess on the tail
	long pos1=tail.load();
	long fcount=0;

	bool WFMode=true;
	bool myOp=false;
	if(dop==NULL){
		WFMode=false;
		myOp=true;
	}

	while(true){//Loop until complete


		if(WFMode){//IF in WF Mode
			DWCAS *o_op=dop->helper.load();
			if(o_op != NULL){//Check if done

				if(myOp){//If this is my OP
					unannounceDelay(); //It is done so take down the announcment
					dop->dealloc();//Dont need the memory, return it to the pool

					if(o_op != (DWCAS*)(0x1)){//Check the state 0x1=Failed because full/empty
						tail.fetch_add(1);//If it passed we enqueued so update tail
						return true;
					}
					else{//Op failed because of size
						return false;
					}
				}
				else{//Not my op but is done so return....
					return false;
				}

			}//End Op is Done
		}//End WF Mode
		else{//Not WF is increment failCount
			if(fcount++ == MAX_FAILURES*capacity){
				WFMode=true;//We reached max failure
				dop=DelayedOp::allocate(this, value);//Get a delay op object
				announceDelayedDequeue(dop);//And Call in the troop
			}
		}

		
		pos1=pos1%capacity;//TODO: optimize if power of two we can shift instead of mod
		void *temp1=buff[pos1].load();

		if(temp1 == tailValue){

			long pos2=(pos1+1)%capacity; //TODO: optimize if power of two we can shift instead of mod
			void *temp2=buff[pos2].load();

			if(temp2 == headValue){//T|H state implies it is full!

				if(WFMode){//If WF mode
					assert(dop != NULL);
					DWCAS *o_op=dop->helper.load();//Check if op is done
					if(o_op == NULL){//If not
						//No Space so set it to failed
						bool passed = dop->helper.compare_exchange_strong(o_op, (DWCAS *)(0x1)); //Set it to failed
						if(passed) o_op= (DWCAS *)(0x1); //if success, set o_op to failed...
					}

					if(myOp){//If my op
						unannounceDelay(); //unannouce
						dop->dealloc(); //dealloc

						if(o_op != (DWCAS*)(0x1)){//Determine if success
							tail.fetch_add(1); //If it passed we enqueued so update tail
							return true;
						}
						else{//Failed (cause of full)
							return false;//Return falls
						}
					}
					else{//Not my op but is done so return....
						return false;
					}
				}
				else{
					return false; //empty buffer
				}
			}
			else if(temp2 == tailValue){//Interesting that this can occur.
				pos1++;//Incrment pos1 and try again, tail has moved!
				continue;
			}


			else if(temp2== notValue){//Valid Sport  Tail|notValue state found
				DWCAS *op=DWCAS::allocate();

				op->reInit(&(buff[pos1]), tailValue, value, //Tail -> value to be enqueued
						   &(buff[pos2]),  notValue, tailValue, //notValue -> Tail
						   dop );

				op->performDWCAS(); //Attempt the OP


				if(WFMode){
					assert(dop != NULL);

					//Check if the the Op is Done
					DWCAS *o_op=dop->helper.load();
					if(o_op != op){//If we did not use op to complete it free it...
						op->dealloc();
					}

					if(o_op != NULL){//If it is done

						if(myOp){//If it is mine clean up
							unannounceDelay();
							dop->dealloc();

							if(o_op != (DWCAS*)(0x1)){
								tail.fetch_add(1);//Success :)
								return true;
							}
							else{ //Failure :( No Space
								return false;
							}
						}
						else{//Not my op but is done so return....
							return false;
						}

					}
					else{//Not done, increment fail count, re-examine state
						fcount=fcount+capacity;
					}

				}
				else{//Not WF mode, so normal op
					bool res= op->getResult();//Lets get the result
					op->dealloc();//We got the result dont need the object anymore

					if(res){//If success
						tail.fetch_add(1); //fix tail
						return true;//return success
					}
					else{//Try agin, re-examine the state
						fcount=fcount+capacity;//Increment fail count significantly

					}
				}//End elsse is not WF Mode

			}//End else attempt DWCAS becase temp2==notValue
			else if(DWCAS::isDWCAS(temp2) ){//It is a helper object
				DWCAS *other=DWCAS::getUnMarkedRef(temp2);

				bool watched=other->watch(&buff[pos2]);//Lets Memory Watch it so we can help
				if(watched){//We got a watch
					other->remove();//Call its remove function
					other->unwatch();//Unwatch
				}//At this point the object has been removed
			}
			else if(Helper::isHelper(temp2) ){
				Helper *other=Helper::getUnMarkedRef(temp2);

				bool watched=other->watch(&buff[pos2]);//Lets Memory Watch it so we can help
				if(watched){//We got a watch
					other->remove(&(buff[pos2]));//Call its remove function
					other->unwatch();//Unwatch
				}//At this point the object has been removed

			}
			else{//it is a valid value.
				pos1++;
			}
		}

		else if(temp1 == headValue || temp1 == notValue){//Not the tail, so lets search for it!
			pos1++; //Search for a spot!
		}
		else if( DWCAS::isDWCAS(temp1) ){//It is a helper object
			DWCAS *other=DWCAS::getUnMarkedRef(temp1);

			bool watched=other->watch(&buff[pos1]);//Lets Memory Watch it so we can help
			if(watched){//We got a watch
				other->remove();//Call its remove function
				other->unwatch();//Unwatch
			}//At this point the object has been removed


		}
		else if(Helper::isHelper(temp1) ){//It is a helper object
			Helper *other=Helper::getUnMarkedRef(temp1);

			bool watched=other->watch(&buff[pos1]);//Lets Memory Watch it so we can help
			if(watched){//We got a watch
				other->remove(&(buff[pos1]));//Call its remove function
				other->unwatch();//Unwatch
			}//At this point the object has been removed

		}
		else{
			pos1++;
		}


	}//End while loop

	assert(false);
};



bool RingBuffer::dequeue(void *&value, DelayedOp *dop  ){
	long pos1=head.load();
	long fcount=0;


	bool myOp=false;
	bool WFMode=true;
	if(dop==NULL){
		myOp=true;
		WFMode=false;
	}

	while(true){

		if(WFMode){
			DWCAS *o_op=dop->helper.load();
			if(o_op != NULL){

				if(myOp){
					unannounceDelay();


					if (o_op != (DWCAS*)(0x1)){
						head.fetch_add(1);

						value=o_op->ev2;
						dop->dealloc();
						return  true;
					}
					else{
						dop->dealloc();
						return false;
					}
				}
				else{
					return false;
				}
			}//End Op is Done
		}//End WF Mode
		else{
			if(fcount++ == MAX_FAILURES*capacity){
				WFMode=true;
				dop=DelayedOp::allocate(this);
				announceDelayedDequeue(dop);
			}
		}

		pos1=pos1%capacity;
		void *temp1=buff[pos1].load();

		if(temp1 == headValue){

			long pos2=(pos1+1)%capacity;
			void *temp2=buff[pos2].load();

			if(temp2 == tailValue){
				if(WFMode){
					assert(dop != NULL);
					DWCAS *o_op=dop->helper.load();
					if(o_op == NULL){
						dop->helper.compare_exchange_strong(o_op, (DWCAS *)(0x1));
						o_op=dop->helper.load();
					}
					if(myOp){
						unannounceDelay();


						if (o_op != (DWCAS*)(0x1)){
							head.fetch_add(1);

							value=o_op->ev2;
							dop->dealloc();
							return  true;
						}
						else{
							dop->dealloc();
							return false;
						}
					}
					
					return false;

				}//End Wait-Free Mode
				else{
					return false; //full buffer
				}
			}
			else if(temp2 == headValue){//Interesting that this can occur.
				pos1++;
				continue;
			}
			else if(temp2 != notValue &&DWCAS::isDWCAS(temp2) ){
				DWCAS *other=DWCAS::getUnMarkedRef(temp2);

				bool watched=other->watch(&buff[pos2]);
				if(watched){
					other->remove();
					other->unwatch();
				}
			}
			else if(temp2 != notValue &&Helper::isHelper(temp2) ){
				Helper *other=Helper::getUnMarkedRef(temp2);

				bool watched=other->watch(&buff[pos2]);
				if(watched){
					other->remove(&(buff[pos2]));
					other->unwatch();
				}


			}
			else{
				DWCAS *op=DWCAS::allocate();

				op->reInit(&(buff[pos1]), headValue, notValue,
									&(buff[pos2]), temp2, headValue, dop );
				op->performDWCAS();

				if(WFMode){
					assert(dop != NULL);
					DWCAS *o_op=dop->helper.load();
					if(o_op != op){
						op->dealloc();
					}

					if(o_op != NULL){
						//It is done

						if(myOp){
							unannounceDelay();


							if (o_op != (DWCAS*)(0x1)){
								head.fetch_add(1);

								value=o_op->ev2;
								dop->dealloc();
								return  true;
							}
							else{
								dop->dealloc();
								return false;
							}
						}

						return false;
					}
					else{
						fcount=fcount+capacity;
					}

				}
				else{
					bool res= op->getResult();
					op->dealloc();

					if(res){
						tail.fetch_add(1);

						value=temp2;
						return true;
					}
					else{
						fcount=fcount+capacity;

					}
				}//End elsse is not WF Mode

			}
		}
		else if(temp1 == tailValue){
			pos1++;
		}
		else if(temp1 != notValue && DWCAS::isDWCAS(temp1) ){
			DWCAS *other=DWCAS::getUnMarkedRef(temp1);

			bool watched=other->watch(&buff[pos1]);
			if(watched){
				other->remove();
				other->unwatch();
			}
		}
		else if(temp1 != notValue && Helper::isHelper(temp1) ){
			Helper *other=Helper::getUnMarkedRef(temp1);

			bool watched=other->watch(&buff[pos1]);
			if(watched){
				other->remove(&(buff[pos1]));
				other->unwatch();
			}
		}
		else{//Scan for Tail Value
			pos1++;
		}
		
		
	}


	assert(false);
};




/**** DELAYEDOP CLASS*****/

/**Constructor **/

RingBuffer::DelayedOp::DelayedOp(RingBuffer *r, void *v){
	value=v;
	rb=r;
	helper.store(NULL);
	rc_count.store(0);
	next=NULL;
}



bool RingBuffer::DelayedOp::watch(std::atomic<long> *a){
	//printf("[%d]\t %p +1\tDO-W1[%d]\t\n", threadID, this, tran_watch);
	rc_count.fetch_add(1);
	long temp=a->load()&(~0x1);

	if(temp == (long)this){
		return true;
	}
	else{
		//printf("[%d]\t %p -1\tDO-W1[%d]\t\n", threadID, this, tran_watch);
		int rc_res=rc_count.fetch_add(-1);
		assert(rc_res>0);
		return false;
	}
};

void RingBuffer::DelayedOp::unwatch(){
	//printf("[%d]\t %p -1\tDO-UW2[%d]\t\n", threadID, this, tran_watch);
	int rc_res=rc_count.fetch_add(-1);
	assert(rc_res>0);
}

bool RingBuffer::DelayedOp::isUnWatched(){
	if(rc_count.load() !=0 ){
		assert(rc_count.load() >=0 );
		return false;
	}
	if(helper.load() != NULL && helper.load() != (DWCAS *)0x1){
		if(! helper.load()->isUnWatched()){
			return false;
		}
	}


	return true;
}

RingBuffer::DelayedOp * RingBuffer::DelayedOp::allocate(RingBuffer *r, void *value)
{


	DelayedOp *res=getFromMemoryPool<DelayedOp>(threadID, tl_DelayedOp, tl_DelayedOp_count, gl_DelayedOp);
	if(res != NULL){

		if(res->helper.load() != NULL && res->helper.load() != (DWCAS *)0x1){
			res->helper.load()->dealloc();
		}

		res->rb=r;
		res->helper.store(NULL);
		res->next=NULL;
		res->value=value;

		assert(res != tl_DelayedOp);
		return res;
	}
	else{
		return new DelayedOp(r, value);
	}

};

RingBuffer::DelayedOp::~DelayedOp(){
	helper.load()->dealloc();
}

void RingBuffer::DelayedOp::dealloc(){
	returnToMemoryPool<DelayedOp>(this, threadID, tl_DelayedOp, tl_DelayedOp_count, gl_DelayedOp);

};

/******End DelayedOP Class ******/


/****** DWCAS Class ****/



void RingBuffer::DWCAS::unwatch(){
	DelayedOp( *temp)=op.load();

	if(temp != NULL){
		int rc_res=temp->rc_count.fetch_add(-1);
		assert(rc_res>0);
	}

	int rc_res=rc_count.fetch_add(-1);
	assert(rc_res>0);

};

bool RingBuffer::DWCAS::watch(std::atomic<void *> *a){
	rc_count.fetch_add(1);
	if(a->load() != this->getMarkedRef()){
		int rc_res=rc_count.fetch_add(-1);
		assert(rc_res>0);
		return false;
	}

	DelayedOp *temp=op;
	if(temp != NULL){
		temp->rc_count.fetch_add(1);
		if(a->load() != this->getMarkedRef()){
			int rc_res=rc_count.fetch_add(-1);
			assert(rc_res>0);
			rc_res=temp->rc_count.fetch_add(-1);
			assert(rc_res>0);
			return false;
		}
	}

	return true;
};


bool RingBuffer::DWCAS::isUnWatched(){
	if(rc_count.load() !=0 ){
		assert(rc_count.load() >=0 );
		return false;
	}
	if(phelper.load() != NULL && phelper.load() != (Helper *)0x1){
		if(!phelper.load()->isUnWatched()){
			return false;
		}
	}

	assert(phelper.load() != NULL);//Makes sure op was set to done
	return true;
}

RingBuffer::DWCAS * RingBuffer::DWCAS::allocate()
{

	DWCAS *res=getFromMemoryPool<DWCAS>(threadID, tl_DWCAS, tl_DWCAS_count, gl_DWCAS );
	if(res == NULL){
		return new DWCAS();
	}
	else{
		if(res->phelper.load() != NULL && res->phelper.load() != (Helper *)0x1){
			res->phelper.load()->dealloc();
		}

		return res;
	}
};

void RingBuffer::DWCAS::dealloc(){
	returnToMemoryPool<DWCAS>(this, threadID, tl_DWCAS, tl_DWCAS_count, gl_DWCAS);
};



RingBuffer::DWCAS::DWCAS(){
	phelper.store(NULL);
	next=NULL;
	rc_count.store(0);
}

void RingBuffer::DWCAS::reInit(std::atomic<void *> *p_a1, void *p_ev1, void *p_nv1,
							   std::atomic<void *> *p_a2, void *p_ev2, void *p_nv2,
							   DelayedOp *p_op){
	a1=p_a1;
	ev1=p_ev1;
	nv1=p_nv1;

	a2=p_a2;
	ev2=p_ev2;
	nv2=p_nv2;

	op=p_op;
	phelper.store(NULL);

};

void RingBuffer::DWCAS::performDWCAS(){

	assert(ev1 == headValue || ev1 == tailValue);
	assert(nv2 == headValue || nv2 == tailValue);
	assert(ev1 != ev2);		assert(ev1 != nv1);
	assert(nv1 != nv2);		assert(ev2 != nv2);

	void *tev1=a1->load();
	if(tev1 == ev1){
		if(a1->compare_exchange_strong(tev1, this->getMarkedRef())){
			remove();
			return;
		}

	}
	phelper.store((Helper *)(0x1));
}

bool RingBuffer::DWCAS::getResult(){
	assert(phelper.load() != NULL);
	if(phelper.load()== (Helper*)(0x1)){
		return false;
	}
	else{
		if(associate())
			return  true;
		else{
			return false;
		}
	}
}
bool RingBuffer::DWCAS::associate(){
	if(op.load() == NULL){
		return true;
	}

	DWCAS *temp=op.load()->helper.load();

	if(temp == NULL){
		op.load()->helper.compare_exchange_strong(temp, this);
	}

	if(temp == this || temp == NULL){
		return  true;
	}
	else{
		return false;
	}
}

/**
 This function removes a DWCAS object that was placed at an address
 It does so by first attempting to complete the DWCAS operation.
 IF the value at a2 is equal to ev2 and the thread succefully placed a "Helper" object at a2
	then it will attempt to associate it with the DWCAS objject
		Next it will check if this DWCAS object is associtated with an En/Dequeue Operation record (returns true if op is NULL)
		If it is, it will replace the Helper object with NV2 and the DWCAS object with NV1
		Otherwise it will replace the helper with EV2 and the DWCAS with EV1

 If it failed to place the helper and the current value is  a helper for this DWCAS
 then it will attempt to associate the Helper
 
 If the associate pointer is 0x1, the the DWCAS object will be reveted back to ev1
 If it is a reference to a helper object then IF associate() returns true, DWCAS will be replaced by NV1 and the associate helper will be replaced by NV2
 Otherwise they are replaced by EV1 and EV2 respectfully
 
 **/

void RingBuffer::DWCAS::remove(){

	Helper *tHelper=phelper.load();

	assert(ev1 == headValue || ev1 == tailValue);
	assert(nv2 == headValue || nv2 == tailValue);
	assert(ev1 != ev2);		assert(ev1 != nv1);
	assert(nv1 != nv2);		assert(ev2 != nv2);

	if(tHelper == NULL){//Check to see if the OP is down

		void *tev2=a2->load();
		if(tev2 == ev2){
			Helper *nHelper= Helper::allocate(ev2, this);

			if(a2->compare_exchange_strong(tev2,nHelper->getMarkedRef() )){//We placed helper
				//Now we need to associate
				phelper.compare_exchange_strong(tHelper, nHelper); //associate dw helper with dwcas

				if(tHelper == (void *)0x1 ){ //Fail to associate because Op was set to failure
					void *temp1=this->getMarkedRef();
					a1->compare_exchange_strong(temp1, ev1);

					temp1=nHelper->getMarkedRef();
					a2->compare_exchange_strong(temp1, ev2);
					nHelper->dealloc();

				}
				else{//OP was success! //Some helper was assocated, ie auccess
					bool success=associate();//Now if it was needed?

					if(success){ //if we associate with the op
						void *temp1=this->getMarkedRef();
						a1->compare_exchange_strong(temp1, nv1);//DWCAS object goes to nv

						temp1=nHelper->getMarkedRef();
						if(tHelper == nHelper || tHelper == NULL){//the one we placed goes to nv2 if we assocatie
							a2->compare_exchange_strong(temp1, nv2);
						}
						else{//otherwise revert
							a2->compare_exchange_strong(temp1, ev2);
							nHelper->dealloc();
						}
					}
					else{//if we failed associate with the op, revert everything back
						void *temp1=this->getMarkedRef();
						a1->compare_exchange_strong(temp1, ev1);

						temp1=nHelper->getMarkedRef();
						a2->compare_exchange_strong(temp1, ev2);

						if(tHelper != nHelper && tHelper != NULL){//the one we placed goes to nv2 if we assocatie
							nHelper->dealloc();
						}
					}

				}
				return;
			}//End We placed helper
			else{//We didn't place the helper
				nHelper->dealloc();
			}
		}//End is a match/we placed a helper
		/*
		 Else current does not match excpected or We failed to place the helper
		 */
		if(tev2 != notValue && tev2 != headValue && tev2 != tailValue && Helper::isHelper(tev2)){//Live Lock Can Occur with outt this check
			Helper *tev2_helper=Helper::getUnMarkedRef(tev2);
			if(tev2_helper->watch(a2)){
				if(tev2_helper->parent == this){
					tev2_helper->remove(a2);
				}
				else{
					//Do Nothing
				}

				tev2_helper->unwatch();
			}


		}

		//The current value does not match the expected AND it is not a helper Object for this DWCAS
		tHelper=NULL;
		bool res=phelper.compare_exchange_strong(tHelper, (Helper *)(0x1));//So Fail it!
		if(res || tHelper == (Helper *)(0x1)){
			void *temp1=this->getMarkedRef();//If success
			a1->compare_exchange_strong(temp1, ev1);///Remove this DWCAS
			return;
		}
	}//End Helper is NULL
	//Someone already placed the helper  And has associated it! How nice

	//So lets cleanup
	if(tHelper == (Helper*)(0x1)){//Failled Op so revert
		void *temp1=this->getMarkedRef();
		a1->compare_exchange_strong(temp1, ev1);
	}
	else{
		if(associate()){//Success to replace with new values
			void *temp1=this->getMarkedRef();
			a1->compare_exchange_strong(temp1, nv1);
			temp1=Helper::getUnMarkedRef(tHelper);
			a2->compare_exchange_strong(temp1, nv2);
		}
		else{//Failed Op So Revert
			void *temp1=this->getMarkedRef();
			a1->compare_exchange_strong(temp1, ev1);
			temp1=Helper::getUnMarkedRef(tHelper);
			a2->compare_exchange_strong(temp1, ev2);
		}
	}

};//End remove();

/****** END DWCAS Class ****/

/********Begin Helper Class********/


RingBuffer::Helper * RingBuffer::Helper::allocate(void *r, DWCAS *p){

	Helper *res=getFromMemoryPool(threadID, tl_Helper, tl_Helper_count, gl_Helper);
	if(res==NULL){
		return new Helper(r, p);
	}
	else{
		res->rvalue=r;
		res->parent=p;
		return res;
	}
};


void RingBuffer::Helper::dealloc(){
	returnToMemoryPool<Helper>(this, threadID, tl_Helper, tl_Helper_count, gl_Helper);
}


void RingBuffer::Helper::unwatch(){
	DWCAS *dw_temp=parent;
	DelayedOp *op_temp=parent->op.load();

	if(op_temp != NULL){
		int rc_res=op_temp->rc_count.fetch_add(-1);
		assert(rc_res>0);
	}

	int rc_res=dw_temp->rc_count.fetch_add(-1);
	assert(rc_res>0);

	rc_res=rc_count.fetch_add(-1);
	assert(rc_res>0);

}
bool RingBuffer::Helper::watch(std::atomic<void *> *a){
	rc_count.fetch_add(1);
	if(a->load() != this->getMarkedRef()){
		int rc_res=rc_count.fetch_add(-1);
		assert(rc_res>0);
		return false;
	}

	DWCAS *tempp=parent;
	tempp->rc_count.fetch_add(1);
	if(a->load() != this->getMarkedRef()){
		int rc_res=rc_count.fetch_add(-1);
		assert(rc_res>0);

		rc_res=tempp->rc_count.fetch_add(-1);
		assert(rc_res>0);
		return false;
	}

	DelayedOp *tempOp=tempp->op.load();
	if(tempOp != NULL){
		tempOp->rc_count.fetch_add(1);
		if(a->load() != this->getMarkedRef()){

			int rc_res=rc_count.fetch_add(-1);
			assert(rc_res>0);

			rc_res=tempp->rc_count.fetch_add(-1);
			assert(rc_res>0);

			rc_res=tempOp->rc_count.fetch_add(-1);
			assert(rc_res>0);
			return false;
		}
	}

	return true;
}


bool RingBuffer::Helper::isUnWatched(){
	if(rc_count.load() != 0){
		assert(rc_count.load() >=0);
		return  false;
	}

	return true;
}

RingBuffer::Helper::Helper(void *r, DWCAS *p){
	rvalue=r;
	parent=p;

	next=NULL;
	rc_count.store(0);
};


void RingBuffer::Helper::remove(std::atomic<void *> *a){
	void *temp=a->load();
	if(temp == this->getMarkedRef()){
		Helper *ptemp=parent->phelper.load();
		if(ptemp==NULL){
			parent->phelper.compare_exchange_strong(ptemp, this);
		}

		if((ptemp == this || ptemp==NULL) && parent->associate()){
			a->compare_exchange_strong(temp, parent->nv2);
		}
		else{
			a->compare_exchange_strong(temp, parent->ev2);
		}

	};

};
/*********End Helper Class ********/





std::atomic<bool> goYo;
int cap=128;

std::atomic<long> e(0x8);
std::atomic<long> d(0x8);

RingBuffer *rb;

void justDequeue();
void justEnqueue();
void queueOps();

/*
int main(int argc, const char * argv[]){

	int mThreads=64;
	Init_RingBuffer_Memory_Management(mThreads+1);
	rb=new RingBuffer(cap);

	goYo.store(true);
	printf("Commence\n");


	std::thread **threads=(std::thread **)malloc(sizeof(std::thread)*mThreads);

	int i;
	for(i=0; i<mThreads; i++){
		threads[i]=new std::thread(queueOps);
	}

	std::this_thread::sleep_for(std::chrono::seconds(10));
	printf("Awakened\n");
	goYo.store(false);

	for(i=0; i<mThreads; i++){
		threads[i]->join();
	}
	printf("Fin\n");

	void *tdequeue=NULL;
	while(rb->dequeue(tdequeue)){
		d.fetch_add(0x8);
	};


	delete rb;
	DESTORY_THREAD_ID();
	Destory_RingBuffer_Memory_Management();


	printf("Enqueues: %ld, Dequeues: %ld\n", e.load(), d.load());

	return 0;


}; */

void queueOps(){
	INIT_THREAD_ID();

	int j;
	int count=0;
	int lecount=0;
	int ldcount=0;
	long temp_e=e.fetch_add(0x8);

	while(goYo.load()){count++;
		int op=rand()%100;
		int range=rand()%cap;

		if(op< 50){
			for(j=0; j<range; j++){
				void *v2=(void *)temp_e;
				bool res=rb->enqueue(v2);
				if(res){
					temp_e=e.fetch_add(0x8);
					lecount++;
				}
			}
		}
		else{
			for(j=0; j<range; j++){
				void *v2=NULL;
				bool res=rb->dequeue(v2);
				if(res){
//					assert((void *)d==v2);
					d.fetch_add(0x8);
					ldcount++;

				}
			}
		}
	}//End While loop

	DESTORY_THREAD_ID();
	printf("Enqueues: %d Dequeues %d DWCAS: %d Helper: %d WFDescr: %d\n",lecount, ldcount, tl_DWCAS_count, tl_Helper_count, tl_DelayedOp_count);

}

void justEnqueue(){
	INIT_THREAD_ID();
	int count=0;

	while(goYo.load()){count++;
		int range=rand()%cap;
		int j;
		for(j=0; j<range; j++){
			void *v2=(void *)(e.load());
			bool res=rb->enqueue(v2);
			if(res){
				e.fetch_add(0x8);
			}
		}

	}

	DESTORY_THREAD_ID();

}

void justDequeue(){
	INIT_THREAD_ID();
	int count=0;
	while(goYo.load()){count++;
		int range=rand()%cap;
		int j;
		for(j=0; j<range; j++){
			void *v2=NULL;
			bool res=rb->dequeue(v2);
			if(res){
				assert((void *)d.load()==v2);
				d.fetch_add(0x8);
			}
		}
	}

	DESTORY_THREAD_ID();
}






