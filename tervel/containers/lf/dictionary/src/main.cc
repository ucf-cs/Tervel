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
#include "dictionary/dictadaptor.h"

ThreadBarrier ba(4);

template<typename T>
void LockThread(uint32_t numThread, int threadId, uint32_t testSize, uint32_t keyRange, uint32_t insertion, uint32_t deletion, ThreadBarrier& barrier,  T& dict)
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
    boost::uniform_int<uint32_t> randomDist(1, keyRange);
    boost::uniform_int<uint32_t> randomDistOp(1, 100);
    
    dict.Init(threadId);

    barrier.Wait();
    
    for(unsigned int i = 0; i < testSize; ++i)
    {
        uint32_t op = randomDistOp(randomGenOp);
        if(op <= insertion)
        {
            dict.Insert(randomDist(randomGen));
        }
        else if(op <= insertion + deletion)
        {
            dict.Delete(randomDist(randomGen));
        }
        else
        {
            dict.Find(randomDist(randomGen));
        }
    }

    dict.Uninit();
}


template<typename T>
void Tester(uint32_t numThread, uint32_t testSize, uint32_t keyRange, uint32_t insertion, uint32_t deletion,  DictAdaptor<T>& dict)
{
    std::vector<std::thread> thread(numThread);
    ThreadBarrier barrier(numThread + 1);

    double startTime = Time::GetWallTime();
    boost::mt19937 randomGen;
    randomGen.seed(startTime - 10);
    boost::uniform_int<uint32_t> randomDist(1, keyRange);

    dict.Init(0);

    for(unsigned int i = 0; i < testSize; ++i)
    {
        dict.Insert(randomDist(randomGen));
    }

    //Create joinable threads
    for (unsigned i = 0; i < numThread; i++) 
    {
        thread[i] = std::thread(LockThread<DictAdaptor<T> >, numThread, i + 1, testSize, keyRange, insertion, deletion, std::ref(barrier), std::ref(dict));
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

    dict.Uninit();
}

int main(int argc, const char *argv[])
{
    uint32_t dictType = 0;
    uint32_t numThread = 4;
    uint32_t testSize = 100000;
    uint32_t keyRange = 100000;
    uint32_t insertion = 50;
    uint32_t deletion = 50;

    if(argc > 1) dictType = atoi(argv[1]);
    if(argc > 2) numThread = atoi(argv[2]);
    if(argc > 3) testSize = atoi(argv[3]);
    if(argc > 4) keyRange = atoi(argv[4]);
    if(argc > 5) insertion = atoi(argv[5]);
    if(argc > 6) deletion = atoi(argv[6]);

    assert(dictType < 7);

    const char* dictName[] = 
    {   "MDList", 
        "CtrDict",
        "FrDict", 
        "DkDict",
        "BrDict", 
        "ElDict", 
        "HlDict"};

    printf("Start testing %s with %d threads %d iterations %d unique keys %d%% insert %d%% delete.\n", dictName[dictType], numThread, testSize, keyRange, insertion, (insertion + deletion) >= 100 ? 100 - insertion : deletion);

    switch(dictType)
    {
    case 0:
        { DictAdaptor<MDList> dict(testSize, numThread + 1, keyRange); Tester(numThread, testSize, keyRange, insertion, deletion, dict); }
        break;
    case 1:
        { DictAdaptor<CTRDICT> dict(numThread + 1); Tester(numThread, testSize, keyRange, insertion, deletion, dict); }
        break;
    case 2:
        { DictAdaptor<FRDICT> dict; Tester(numThread, testSize, keyRange, insertion, deletion, dict); }
        break;
    case 3:
        { DictAdaptor<DKDICT> dict; Tester(numThread, testSize, keyRange, insertion, deletion, dict); }
        break;
    case 4:
        { DictAdaptor<BRDICT> dict; Tester(numThread, testSize, keyRange, insertion, deletion, dict); }
        break;
    case 5:
        { DictAdaptor<ELDICT> dict; Tester(numThread, testSize, keyRange, insertion, deletion, dict); }
        break;
    case 6:
        { DictAdaptor<HLDICT> dict; Tester(numThread, testSize, keyRange, insertion, deletion, dict); }
        break;
    //case 7:
        //{ DictAdaptor<HLODICT> dict; Tester(numThread, testSize, keyRange, insertion, deletion, dict); }
        //break;
    //case 3:
        //{ DictAdaptor<TBBMAP> dict; Tester(numThread, testSize, keyRange, insertion, deletion, dict); }
        //break;
    //case 4:
        //{ DictAdaptor<STDMAP> dict; Tester(numThread, testSize, keyRange, insertion, deletion, dict); }
        //break;
    default:
        break;
    }

    return 0;
}
