//------------------------------------------------------------------------------
// 
//     Testing different priority queues
//
//------------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <set>
#include <thread>
#include <mutex>
#include <boost/random.hpp>
#include <sched.h>
#include "common/timehelper.h"
#include "common/threadbarrier.h"
#include "priorityqueue/queueadaptor.h"

ThreadBarrier ba(4);

template<typename T>
void LockThread(uint32_t numThread, int threadId, uint32_t testSize, uint32_t portion, ThreadBarrier& barrier,  T& pq)
{
    //set affinity for each thread
    cpu_set_t cpu = {{0}};
    CPU_SET(threadId, &cpu);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpu);

    double startTime = Time::GetWallTime();
    boost::mt19937 randomGen;
    boost::mt19937 randomGenOp;
    randomGen.seed(startTime + threadId);
    randomGenOp.seed(startTime + threadId + 1000);
    boost::uniform_int<uint32_t> randomDist(1, 0xfffffffa);
    boost::uniform_int<uint32_t> randomDistOp(1, 100);
    
    pq.Init();

    barrier.Wait();

    for(unsigned int i = 0; i < testSize / numThread; ++i)
    //for(unsigned int i = 0; i < testSize; ++i)
    {
        if(randomDistOp(randomGenOp) <= portion)
        {
            pq.Insert(randomDist(randomGen));
        }
        else
        {
            pq.DeleteMin();
        }
    }

    //for(unsigned int i = 0; i < testSize; ++i)
    //{
        //pq.Insert(randomDist(randomGen));
    //}    

    //ba.Wait();

    //for(unsigned int i = 0; i < testSize; ++i)
    //{
        //pq.DeleteMin();
    //}

    pq.Uninit();
}

template<typename T>
void Tester(uint32_t numThread, uint32_t testSize, uint32_t portion,  QueueAdaptor<T>& pq)
{
    std::vector<std::thread> thread(numThread);
    ThreadBarrier barrier(numThread + 1);

    double startTime = Time::GetWallTime();
    boost::mt19937 randomGen;
    randomGen.seed(startTime - 10);
    boost::uniform_int<uint32_t> randomDist(1, 0xfffffffa);

    pq.Init();

    for(unsigned int i = 0; i < testSize; ++i)
    {
        pq.Insert(randomDist(randomGen));
    }

    //Create joinable threads
    for (unsigned i = 0; i < numThread; i++) 
    {
        thread[i] = std::thread(LockThread<QueueAdaptor<T> >, numThread, i, testSize, portion, std::ref(barrier), std::ref(pq));
    }

    barrier.Wait();

    {
        ScopedTimer timer(true);

        //Wait for the threads to finish
        for (unsigned i = 0; i < thread.size(); i++) 
        {
            thread[i].join();
        }
    }

    pq.Uninit();
}

int main(int argc, const char *argv[])
{
    uint32_t queueType = 0;
    uint32_t numThread = 4;
    uint32_t testSize = 100000;
    uint32_t portion = 50;

    if(argc > 1) queueType = atoi(argv[1]);
    if(argc > 2) numThread = atoi(argv[2]);
    if(argc > 3) testSize = atoi(argv[3]);
    if(argc > 4) portion = atoi(argv[4]);

    assert(queueType < 4);

    const char* queueName[] = {"MDPQ", "LJPQ", "TBBPQ", "HSPQ", "STDPD"};

    printf("Start testing %s with %d threads %d iterations %d%% insert.\n", queueName[queueType], numThread, testSize, portion);

    switch(queueType)
    {
    case 0:
        { QueueAdaptor<MDList> pq(testSize, numThread + 1); Tester(numThread, testSize, portion, pq); }
        break;
    case 1:
        { QueueAdaptor<LINDENPQ> pq; Tester(numThread, testSize, portion, pq); }
        break;
    case 2:
        { QueueAdaptor<TBBPQ> pq; Tester(numThread, testSize, portion, pq); }
        break;
    case 3:
        { QueueAdaptor<SHAVITPQ> pq; Tester(numThread, testSize, portion, pq); }
        break;
    case 4:
        { QueueAdaptor<STDPQ> pq; Tester(numThread, testSize, portion, pq); }
        break;
    default:
        break;
    }

    return 0;
}
