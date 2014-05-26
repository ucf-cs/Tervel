#include <limits>  
#include <iostream>

#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>

#include "bufferAPI.h"

volatile bool wait;
volatile bool stop;
int nThreads, capacity, enqRate, deqRate, prefill;

Buffer buffer;

struct counter
{
	int enq=0;
	int deq=0;
	int enqSuc=0;
	int deqSuc=0;
	void init() {
        int enq=0;
        int deq=0;
        int enqSuc=0;
        int deqSuc=0;
	}

	long total(){
		return enq+deq+enqSuc+deqSuc;
	}

	void print(void * tid){
		int sum=prefill+enqSuc-deqSuc;
		int total=this->total();
		printf("[%p] Enqueues: %d, Dequeues: %d, Enqueue on Full: %d, Dequeue on Empty: %d. == %d. [net size %d] \n", tid, enq, deq, enqSuc, deqSuc, total, sum);
	}
	void add(struct counter *c){
		enq+=c->enq;
		deq+=c->deq;
		enqSuc+=c->enqSuc;
		deqSuc+=c->deqSuc;
	}

}counter;

void* runThread( void *tid){

	buffer.q_init_thread();
 
	struct counter *c=(struct counter *)malloc(sizeof(struct counter));
	
	long vcount=(0x100 + 0x10*(long)tid);

	boost::mt19937 rng((long)tid);
	boost::uniform_int<> opRand(1, 100);
	boost::uniform_int<> posRand(0,  std::numeric_limits<int>::max());

	int pos;

	while(wait){};
	while(!stop){

		int op=opRand(rng);

		if( op <= enqRate) {
			Node *n = new Node(vcount);
            Node *res = buffer.q_enqueue(n, (long) tid); //buff.enqueue(n, 0);
            vcount=vcount+0x10*nThreads;
            
            c->enq++;
            if(n == res) {
                c->enqSuc++;
                //enqSuc++;
            } else if (res == (Node*) 0x1) {
            
            } else if (res == (Node*) 0x2) {
             
            } else{ // (n != res): failed enqueue
                     // failed to enqueue   
            }
		} else if( op <= enqRate+deqRate) {
            Node *n = buffer.q_dequeue((long) tid); //buff.dequeue(0);
            vcount=vcount+0x10*nThreads;
            
            c->deq++;
            if (n == (Node*) 0x1) {
             
            } else if (n == (Node*) 0x2) {
             
            } else if (n != NULL) {
                c->deqSuc++;
            } else { // (n == NULL): failed dequeue
                // failed to dequeue
            }
		} else{
			printf("Error: op==%d \n", op);
			assert(false);
			
			return (void *)std::numeric_limits<long>::min();
		}
		
	}

	buffer.q_finish_thread();


	//c->print(tid);
	
	long t=c->total();

	free(c);
	return (void *)t;
};



int main(int argc, char **argv){
	int apos=1;
	int exeTime;
	
    if(argc == 7){
		exeTime=atoi(argv[apos++]);
		nThreads=atoi(argv[apos++]);
	    capacity=atoi(argv[apos++]);
        
		enqRate=atoi(argv[apos++]);
		deqRate=atoi(argv[apos++]);
		
        prefill=atoi(argv[apos++]);

        #ifdef NO_ENQUEUE
        if(enqRate !=0){
			printf("%ld", 0L);
			return -1;
		}
        #endif

        #ifdef NO_DEQUEUE
        if(deqRate !=0){
			printf("%ld", 0L);
			return -1;
		}
        #endif
	
    //	printf("Time: %d NThreads: %d CasRate: %d GetRate: %d: PopRate: %d PushRate: %d InsertRate: %d EraseRate: %d prefill: %d\n", exeTime, nThreads, casRate, getRate, popRate, pushRate, insertRate, eraseRate, prefill);
		int tsum=enqRate+ deqRate;
		if(tsum != 100){
			printf("Propbabilities must sum to equal 100 (Current Sum: %d)",tsum);
				printf("%ld", -1L);
			return -1;
		}
	}
	else{
		printf("Order: Time, NThreads, capacity EnqRate, DeqRate, prefill(%d args)\n",argc);
		int i;
		for(i=0; i<argc; i++){
			printf("[%d] %s\n",i, argv[i]);
			
		}
		return -1;
	}	
	
	wait=true;
	stop=false;

    buffer.q_init(nThreads+1, capacity);
	
	long vcount=0x10;

	int p;
	for(p=0; p<prefill; p++) {
        Node *node = new Node(vcount);
		buffer.q_enqueue(node, 0);
		vcount=vcount+0x10;
	}

	pthread_t *thread_list=(pthread_t *)malloc(sizeof(pthread_t)*nThreads);

	//printf("Commencing\n");
	long t=0;
	for(t=0; t<nThreads; t++){
		pthread_create( &thread_list[t], NULL,runThread, (void *)(t+1));
	}
	//printf("Sleeping\n");

	wait=false;
	sleep(exeTime);

	//printf("Awakened\n");
	stop=true;
	long total=0;
	for(t=0; t<nThreads; t++){
		void *temp;
		pthread_join(thread_list[t],&temp);
		total=total+(long)temp;
	}

	printf("%ld", total);
	buffer.q_finish();
	
	return total;
	
};
