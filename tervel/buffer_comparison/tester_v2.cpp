#include <limits>  
#include <iostream>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <unistd.h>
#include <thread>
#include <assert.h>
#include "bufferAPI.h"
#include <exception>

#include <chrono>

#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>

volatile bool wait;
volatile bool stop;
Buffer buffer;

int nThreads;
typedef struct dists{
	int enqRate, deqRate;

	void print(){
		printf("Enqueues: %d, Dequeues: %d:\n",enqRate, deqRate);
	}
}dists;

typedef struct Counter
{
	int enqs=0;
	int deqs=0;
	int succEnqs=0;
	int succDeqs=0;
	void init(){
        enqs=0;
        deqs=0;
        succEnqs=0;
        succDeqs=0;
	}

	long total(){
		return enqs+deqs;
	}

	void print_result(){
		printf("[%d, %d]", succEnqs, succDeqs);
	}
	void print(void * tid){
		int sum= succEnqs-succDeqs;
		int total=enqs+deqs;
		printf("[%p] Enqueues: %d, Dequeues: %d, Successful Enqueuess: %d, Successful Dequeues: %d: == %d. [net size %d] \n", tid, enqs, deqs, succEnqs, succDeqs, total, sum);
	}
	void add(Counter *c){
		assert(c!= NULL);
		enqs+=c->enqs;
		deqs+=c->deqs;
		succEnqs+=c->succEnqs;
		succDeqs+=c->succDeqs;
	}

}Counter;

void runThread( void *tid, dists *d, Counter **cres){

	buffer.q_init_thread();
    int thrID = *((int*)(&tid));
 
	Counter *c=(Counter *)malloc(sizeof(Counter));
	c->init();
	
	long vcount=(0x100 + 0x10*(long)tid);

	boost::mt19937 rng((long)tid);
	boost::uniform_int<> opRand(1, 100);
	boost::uniform_int<> posRand(0,  std::numeric_limits<int>::max());

	int enqRate=d->enqRate, deqRate=d->deqRate;

	int pos;

	while(wait){};
	while(!stop){

		int op;
        #ifdef USE_LINUX_BUFFER
            op=(thrID%2)+enqRate;
        #else
            op=opRand(rng);
        #endif
        
        if( op <= enqRate) {
            Node *n = new Node(vcount);
            bool succ = buffer.q_enqueue(n, (long) tid); //buff.enqueue(n, 0);
            vcount=vcount+0x10*nThreads;
     
            if(succ) {
                c->succEnqs++;
                //enqSuc++;
            } else{ 
                // failed to enqueue   
            }
        } else if( op <= enqRate+deqRate) {
            bool succ = buffer.q_dequeue((long) tid); //buff.dequeue(0);
            vcount=vcount+0x10*nThreads;
    
            if (succ) {
                c->succDeqs++;
            } else {
                // failed to dequeue
            }
		} else {
		    printf("Error: op==%d \n", op);
		    assert(false);
		    exit(-1);
		    return;
		}
	}

	buffer.q_finish_thread();
	*cres=c;
	return;
};



int main(int argc, char **argv){
	int apos=1;

	if(argc != 8){
		printf("Order: Time, size, prefll, Total Threads, [NThreads, EnqRate, DeqRate]* (%d args)\n",argc);
		int i;
		for(i=0; i<argc; i++){
			printf("[%d] %s\n",i, argv[i]);
			
		}
		return -1;
	}

	int exeTime=atoi(argv[apos++]);
	int size=atoi(argv[apos++]);
	int prefill=atoi(argv[apos++]);
	int nTotalThreads=atoi(argv[apos++]);
 //   nThreads = nTotalThreads;

	std::thread **threads=new std::thread *[nTotalThreads];
	Counter** counters=new Counter*[nTotalThreads];

	
	buffer.q_init(nTotalThreads+1, size);
	
	int p; long vcount=0x10;
    buffer.q_init_thread();
	for(p=0; p<prefill; p++){
        Node *node = new Node(vcount);
		buffer.q_enqueue(node, 0);
		vcount=vcount+0x10;
	}
    //buffer.q_finish_thread();

	wait=true;
	stop=false;

	long i=0;
	while(apos <argc){ 
		dists *d=new dists;

		int nthreads=atoi(argv[apos++]);
		d->enqRate=atoi(argv[apos++]);
		d->deqRate=atoi(argv[apos++]);

        #ifdef NO_ENQ
	        if(d->enqRate !=0){
				printf("%ld", 0L);
				return -1;
			}
        #endif

        #ifdef NO_DEQ
	        if(d->deqRate !=0){
				printf("%ld", 0L);
				return -1;
			}
        #endif
	//	printf("Time: %d NThreads: %d CasRate: %d GetRate: %d: PopRate: %d PushRate: %d InsertRate: %d EraseRate: %d prefill: %d\n", exeTime, nThreads, casRate, getRate, popRate, pushRate, insertRate, eraseRate, prefill);
		int tsum=d->enqRate+ d->deqRate;
		if(tsum != 100){
			printf("Propbabilities must sum to equal 100 (Current Sum: %d)",tsum);
				printf("%ld", -1L);
			return -1;
		}

		int j;
		for(j=0; j<nthreads; j++){
			if(i==nTotalThreads){
				printf("nTotalThreads is less than specified threads\n");
				return -1;
			}
			threads[i]=new std::thread(runThread, (void *)i, d, &counters[i]);
			assert(threads[i] != NULL);
			i++;

		}
		//d->print();
	}
	assert(i==nTotalThreads);
	
	wait=false;
	std::this_thread::sleep_for(std::chrono::seconds(exeTime));

	//printf("Awakened\n");
	stop=true;

	std::this_thread::sleep_for(std::chrono::seconds(1));

	long total=0;

	Counter totalOps;
	totalOps.init();
	int t;
	for(t=0; t<nTotalThreads; t++){
		try{
			assert(threads[t] != NULL);
			threads[t]->join();
		}
		catch (std::exception& e){
    	//	std::coutl << e.what() << '\n';
  		}
		totalOps.add(counters[t]);
	}

	totalOps.print_result();

	buffer.q_finish();
	
	return total;
	
};
