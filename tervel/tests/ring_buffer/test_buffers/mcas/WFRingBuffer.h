//
//  WFRingBuffer.h
//  WF RingBuffer
//
//  Created by Steven Feldman on 10/29/13.
//  Copyright (c) 2013 Steven FELDMAN. All rights reserved.
//

#ifndef __WF_RingBuffer__WFRingBuffer__
#define __WF_RingBuffer__WFRingBuffer__

#include <iostream>
#include <atomic>
#include <assert.h>
#include <utility>
#include <thread>
#include <chrono>

#define  notValue  (void *)0x1
#define  headValue (void *)0x2
#define  tailValue (void *)0x3


void Init_RingBuffer_Memory_Management(int _nthreads);

class RingBuffer {
public:
	RingBuffer (size_t s);
	~RingBuffer();

	bool enqueue(void *&value ){
		assert(value != headValue || value != tailValue || value != notValue);
		assert(!DWCAS::isDWCAS(value));
		assert(!Helper::isHelper(value));

		return enqueue(value, NULL);
	}
	bool dequeue(void *&value ){
		return dequeue(value, NULL);
	};

	size_t getCapacity(){
		return capacity;
	};

	void printQueue(){
		long i;
		for(i=0; i< capacity; i++){
			long cvalue=(long)buff[i].load();
			if(cvalue == (long)headValue){
				printf("[%ld, TAIL(%ld) ] ", i, cvalue);

			}
			else if(cvalue ==  (long)tailValue){
				printf("[%ld, HEAD(%ld) ] ", i, cvalue);

			}
			else
				printf("[%ld, %ld] ", i, cvalue);
		}
		printf("\n");
	};


	class DWCAS;
	class Helper;
	class DelayedOp;




private:
	size_t capacity;
	std::atomic<long> head;
	std::atomic<long> tail;
	std::atomic<void *> *buff;
	void announceDelayedEnqueue(RingBuffer::DelayedOp *dop);
	void announceDelayedDequeue(RingBuffer::DelayedOp *dop);
	void unannounceDelay();
	void helpDelayed();
	bool enqueue(void *value, DelayedOp *op);
	bool dequeue(void *&value, DelayedOp *op);



public:
	class DelayedOp{
	public:
		RingBuffer *rb;
		void *value;
		std::atomic<DWCAS* > helper;

		DelayedOp(RingBuffer *rb, void *v=NULL);
		~DelayedOp();
		//Memory Management Functions
		static DelayedOp * allocate(RingBuffer *rb, void *value=NULL);
		void dealloc();

		std::atomic<int> rc_count;
		DelayedOp *next;

		bool watch(std::atomic<long> *a);
		void unwatch();
		bool isUnWatched();


	};


	/**
	 This Class is used to perform a DWCAS opertation
		It is specilized to also allow for the association with an Delayed OP object
	 
	 **/
	class DWCAS{
public:
		
		std::atomic<void *> *a1; void *ev1; void *nv1;
		std::atomic<void *> *a2; void *ev2; void *nv2;
		std::atomic<Helper *> phelper;
		std::atomic<DelayedOp *> op;

		DWCAS();
		void reInit(std::atomic<void *> *p_a1, void *p_ev1, void *p_nv1,
					std::atomic<void *> *p_a2, void *p_ev2, void *p_nv2, DelayedOp *p_op);


		void performDWCAS();
		bool getResult();
		bool associate();
		void remove();

		//Memory management Functions
		static DWCAS * allocate();
		void dealloc();

		std::atomic<int> rc_count;
		DWCAS * next;

		bool isUnWatched();
		bool watch(std::atomic<void *> *a);
		void unwatch();


		//Type Identification Functions
		static bool isDWCAS(void *value){
			assert(value != notValue && value != headValue && value != tailValue);
			return ((long)value&(0x1)) != 0L;
		};
		static DWCAS * getUnMarkedRef(void *value){
			assert(value != notValue && value != headValue && value != tailValue);
			return (DWCAS *)( ((long)value & (~ 0x1)) );
		};

		void * getMarkedRef(){
			assert(this != notValue && this != headValue && this != tailValue);
			return (void *)( ((long)(this) | (  0x1)) );
		}

	};//End DWCAS Class



	/**
	 This Class is used to complete the Second Phase of a Software Based Double-Word CAS
	 It references a DWCAS object that has been placed at an address(parent)
	 and a thread will attempt to place this at the second address in the operation and set parent's helper pointer to it.

	 It can only be removed after being placed after parent's helper pointer has been set and if parent is references an op record, after that op records helper pointer has also been set.

	 **/

	class Helper{
	public:
		Helper(void *r, DWCAS *p);
		void *rvalue;
		DWCAS *parent;

		void remove(std::atomic<void *> *a);

		//Memory Management Functions/Variables
		std::atomic<int> rc_count;
		Helper * next;
		static Helper * allocate(void *r, DWCAS *p);
		void dealloc();

		bool isUnWatched();
		bool watch(std::atomic<void *> *a);
		void unwatch();

		//Type Identification functions
		static bool isHelper(void *value){
			assert(value != notValue && value != headValue && value != tailValue);
			return ((long)value&(0x2)) != 0L;
		};
		static Helper * getUnMarkedRef(void *value){
			assert(value != notValue && value != headValue && value != tailValue);
			return (Helper *)( ((long)value & (~ 0x2)) );
		};

		void * getMarkedRef(){
			assert(this != notValue && this != headValue && this != tailValue);
			return (void *)( ((long)(this) | (  0x2)) );
		}
		
		
	};

};


#include "WFRingBuffer.hpp"

#endif /* defined(__WF_RingBuffer__WFRingBuffer__) */
