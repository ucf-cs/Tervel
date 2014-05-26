#include <limits>  
#include <iostream>

#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include "bufferAPI.h"

#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>

volatile bool wait;
volatile bool stop;
int nThreads, capacity, enqRate, deqRate;

Buffer buffer;
//WFRingBuffer buff;
//OnlyAtomicFAA buff;

void* runThread( void *tid){
	
    buffer.q_init_thread();

	long count=0;
	long enqCount=0;
	long deqCount=0;
	
	long vcount=(0x100 + 0x10*(long)tid);

	boost::mt19937 rng((long)tid);
	boost::uniform_int<> opRand(1, 100);
	boost::uniform_int<> posRand(0,  std::numeric_limits<int>::max());
	int pos;

    while(wait){};
	
    while(!stop){
		count++;
#if 0
		int res=p_pushBack((void *)vcount);
		count++;
		if(res != -1){
			vcount=vcount+0x10*nThreads;
		}
		else{
			assert(false);
		}
		
		void *temp;
		bool res2=p_popBack();
		
#else
		
		int op=opRand(rng);
        /*
        if (((long) tid) == 1)
            op = 0;
        else if (((long) tid) == 2)
            op = deqRate+enqRate;
        */

		if( op <= enqRate){
            Node *n = new Node(vcount);
            Node *res = buffer.q_enqueue(n, (long) tid); //buff.enqueue(n, 0);
			
            if(n == res){
                vcount=vcount+0x10*nThreads;
                enqCount++;
                //enqSuc++;
            } else if (res == (Node*) 0x1) {
            
            } else if (res == (Node*) 0x2) {
            
            } else{ // (n != res): failed enqueue
                // failed to enqueue   
			}
		}
		else if( op <= deqRate+enqRate){
			//deqCount++;
            Node *n = buffer.q_dequeue((long) tid); //buff.dequeue(0);
            if (n == (Node*) 0x1) {
            
            } else if (n == (Node*) 0x2) {
            
            } else if (n != NULL) {
                deqCount++;
				vcount=vcount+0x10*nThreads;
                //deqSuc++;
            } else { // (n == NULL): failed dequeue
                // failed to dequeue
            }
		}
		else{
			printf("Error: op==%d \n", op);
			assert(false);
			
			count= std::numeric_limits<int>::min();
			break;
		}
		
#endif	
	}

	buffer.q_finish_thread();

/*	printf("%p GC:  %d\n",tid, getCount);
	printf("%p CC:  %d\n",tid, casCount);
	printf("%p PoB: %d\n",tid, popBackCount);
	printf("%p PuB: %d\n",tid, pushBackCount);
	printf("%p IA:  %d\n",tid, insertAtCount);
	printf("%p EA:  %d\n",tid, eraseAtCount);*/

    return (void *) count;
	//return (void *)enqCount+deqCount;
};


int main(int argc, char **argv){
	int apos=1;
	int exeTime;
	int prefill=0;
	if(argc == 7){
		exeTime=atoi(argv[apos++]);
		nThreads=atoi(argv[apos++]);
        capacity=atoi(argv[apos++]);
		
		enqRate=atoi(argv[apos++]);
	    deqRate=atoi(argv[apos++]);
		
        prefill=atoi(argv[apos++]);

        #ifdef NO_INSERT
        if(enqRate !=0){
			printf("%ld", 0L);
			return -1;
		}
        #endif
        #ifdef NO_POP
        if(deqRate !=0){
			printf("%ld", 0L);
			return -1;
		}
        #endif

	//	printf("Time: %d NThreads: %d CasRate: %d GetRate: %d: PopRate: %d PushRate: %d InsertRate: %d EraseRate: %d prefill: %d\n", exeTime, nThreads, casRate, getRate, popRate, pushRate, insertRate, eraseRate, prefill);
		int tsum=enqRate+ deqRate;
		if(tsum != 100){
			printf("Probabilities must sum to equal 100 (Current Sum: %d)",tsum);
			printf("%ld", -1L);
			return -1;
		}
	}
	else{
		printf("Order: Time, NThreads, capacity, EnqRate, DeqRate, prefill(%d args)\n",argc);
		int i;
		for(i=0; i<argc; i++){
			printf("[%d] %s\n",i, argv[i]);
			
		}
		return -1;
	}	

    //printf("Time(%d) threads(%d) capacity(%d) enqRate(%d) deqRate(%d) prefill(%d)\n", exeTime, nThreads, capacity, enqRate, deqRate, prefill);
	wait=true;
	stop=false;

    buffer.q_init(nThreads + 1, capacity);
    //buff.init(nThreads, capacity);
	//buff.init(nThreads+1);
	
	long vcount=0x10;

	int p;
	for(p=0; p<prefill; p++){
        Node *foo = new Node(vcount);
		buffer.q_enqueue(foo, 0);
        //buff.enqueue(new Node(vcount), 0);
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

    buffer.q_finish();
    
    printf("%ld", total);
	//printf("Ops Completed: %ld \n",total);
	//printf("Ops Completed: %ld  Size: %d \n",total, p_getSize());
	
	return total;
};
