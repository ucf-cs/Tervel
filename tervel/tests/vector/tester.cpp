<<<<<<< HEAD
#include <limits>  
=======
#include <limits>
>>>>>>> master
#include <iostream>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include "vectorAPI.h"

#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>

std::atomic<bool> wait;
std::atomic<bool> stop;
int nThreads;

struct counter
{
	int pushes=0;
	int pops=0;
	int erases=0;
	int inserts=0;
	int reads=0;
	int writes=0;
	void init(){
		pushes=0;
		pops=0;
		inserts=0;
		erases=0;
		reads=0;
		writes=0;

	}

	void print(void * tid){
		int sum= pushes-pops-erases+inserts;
		int total=pushes+pops+inserts+erases+reads+writes;
		printf("[%p] Inserts: %d, Erases: %d, Pushes: %d, Pops: %d, Reads: %d, Writes: %d: == %d. [net size %d] \n", tid, inserts, erases, pushes, pops, reads, writes, total, sum);
	}
	void add(struct counter *c){
		pushes+=c->pushes;
		pops+=c->pops;
		erases+=c->erases;
		inserts+=c->inserts;
		reads+=c->reads;
		writes+=c->writes;
	}

}counter;

void * runThread( void *tid) {

	struct counter *c=(struct counter *)malloc(sizeof(struct counter));
	c->init();
	boost::mt19937 rng((long)tid);
	boost::uniform_int<> opRand(0, 5);
	boost::uniform_int<> posRand(0,  std::numeric_limits<int>::max());
	p_attachThread();

<<<<<<< HEAD
	
	long count=0;
	
	long vcount=(0x100 + 0x10*(long)tid);

	
=======

	long count=0;

	long vcount=(0x100 + 0x10*(long)tid);


>>>>>>> master
	while (wait.load()) {};


	while (!stop.load()) {
<<<<<<< HEAD
		
		//p_printVector();
		

		int op=opRand(rng);

		int opId=0;	
=======

		//p_printVector();


		int op=opRand(rng);

		int opId=0;
>>>>>>> master

		if (op==opId++) {
			p_pushBack((void *)vcount);
		//	printf("----Pushing: %p.\n\n", (void *)vcount);
			c->pushes++;
		}

		else if (op==opId++) {
			void * temp=p_popBack();
		//	printf("-----Popped : %p.\n\n", temp);
			if(temp != NULL)
				c->pops++;
		}
		else if (op==opId++) {
			int s=p_getSize();
			if (s==0) {
				c->pushes++;
				p_pushBack((void *)vcount);
				//printf("Pushing: %p.\n", (void *)vcount);
			}
			else {
				int pos=posRand(rng)%s;
<<<<<<< HEAD
				bool res=p_insertAt(pos, (void *)vcount);	
=======
				bool res=p_insertAt(pos, (void *)vcount);
>>>>>>> master
				if(res){
					c->inserts++;
				}
				//printf("Inserting: %p at %d.\n", (void *)vcount, pos);
			}
		}
		else if (op==opId++) {
			int s=p_getSize();
			if (s==0) {
				c->pushes++;
				p_pushBack((void *)vcount);
				//printf("Pushing: %p.\n", (void *)vcount);
			}
			else {
<<<<<<< HEAD
				
				int pos=posRand(rng)%s;
				void *temp=NULL;;
				p_eraseAt(pos, temp);	
=======

				int pos=posRand(rng)%s;
				void *temp=NULL;;
				p_eraseAt(pos, temp);
>>>>>>> master
				if(temp != NULL)
					c->erases++;
				//printf("Erased : %p at %d.\n", temp, pos);
			}
		}
<<<<<<< HEAD
		
=======

>>>>>>> master
		else if (op==opId++) {

			int s=p_getSize();
			if (s==0) {
				c->pushes++;
				p_pushBack((void *)vcount);
			}
			else {
				int pos=posRand(rng)%s;
				p_readVector(pos);
				c->reads++;
			}
		}
		else if (op==opId++) {
			int s=p_getSize();
			if (s==0) {
				c->pushes++;
				p_pushBack((void *)vcount);
			}
			else {
				int pos=posRand(rng)%s;
				void *expected=p_readVector(pos);
				if(expected!=NULL){
					p_casVector(pos,expected,(void *)vcount);
				}
				c->writes++;
			}
		}
		else{
			assert(false);
		}

<<<<<<< HEAD
		
		count++;
		vcount=vcount+0x10*nThreads;
		
=======

		count++;
		vcount=vcount+0x10*nThreads;

>>>>>>> master
	}

	p_dettachThread();

	//c->print(tid, count);
	//fflush(stdout);

	return c;
};

<<<<<<< HEAD
/*
void* runPushOnly( void *tid) {

	p_attachThread();

	while(wait) {};
	long count=0;
	
	long vcount=(0x100 + 0x10*(long)tid);

	int pos;
	while(!stop) {

		int res=p_pushBack((void *)vcount);
		if(res != -1) {
			vcount=vcount+0x10*nThreads;
		}
		else{
			assert(false);
		}
		
		
		count++;
	}

	p_dettachThread();


	return (void *)count;
};


void* runPopOnly( void *tid) {

	p_attachThread();

	while(wait) {};
	long count=0;
	
	while(!stop) {

		void * res=p_popBack();
		if(res != NULL) {
			count++;
		}
		else{
			break;
		}
		
	}

	p_dettachThread();


	return (void *)count;
};
*/

=======
>>>>>>> master
int main(int argc, char **argv) {
	int apos=1;
	int exeTime;
	if(argc == 3) {
		exeTime=atoi(argv[apos++]);
		nThreads=atoi(argv[apos++]);
<<<<<<< HEAD
		
=======

>>>>>>> master
	}
	else{
		printf("Order: Time, NThreads (%d args)\n",argc);
		int i;
		for (i=0; i<argc; i++) {
			printf("[%d] %s\n",i, argv[i]);
<<<<<<< HEAD
			
		}
		return -1;
	}	
	
=======

		}
		return -1;
	}

>>>>>>> master
	int pfill=100;

	int size=1024;
	p_initVector(size, nThreads+1);

	long value=(0x10);

	int i;
	for(i=0; i< pfill; i++){
		value=value+0x10;
		p_pushBack((void *)value);
	}
<<<<<<< HEAD
	
	
=======


>>>>>>> master

	pthread_t *thread_list=(pthread_t *)malloc(sizeof(pthread_t)*nThreads);

	printf("Commencing\n");
	//p_printVector();
	long t=0;
<<<<<<< HEAD
	wait.store(true); 
=======
	wait.store(true);
>>>>>>> master
	stop.store(false);

	for (t=0; t<nThreads; t++) {
		pthread_create( &thread_list[t], NULL,runThread, (void *)(t+1));
	}
	printf("Sleeping\n");

	wait.store(false);
	sleep(exeTime);
	stop.store(true);
	printf("Awakened\n");
<<<<<<< HEAD
	
=======

>>>>>>> master
	//long total=0;

	struct counter *c=(struct counter *)malloc(sizeof(struct counter));;
	c->init();
	c->pushes+=pfill;

	for (t=0; t<nThreads; t++) {
		void *temp=NULL;
		pthread_join(thread_list[t],&temp);
		c->add((struct counter *)temp);
	}

	p_printVector();
	c->print(0);
	printf("Vec Size: %d .\n", p_getSize());
<<<<<<< HEAD
	
	p_destroyVector();
	
	return 0;
	
=======

	p_destroyVector();

	return 0;

>>>>>>> master
};
