#ifndef QUEUETESTER_H
#define QUEUETESTER_H

#include <tbb/concurrent_priority_queue.h>
#include <queue>
#include "priorityqueue/binomial/binomialheap.h"
#include "priorityqueue/binomial/bheap.h"
#include "priorityqueue/monoheap/monoheap.h"
#include "priorityqueue/vectorheap/vheap.h"
#include "priorityqueue/mdlist/mdlist.h"
//#include "priorityqueue/tbb/concurrent_priority_queue.h"

extern "C"
{
#include "priorityqueue/linden/gc/gc.h"
#include "priorityqueue/linden/prioq.h"
#include "priorityqueue/shavit/set.h"
}

template<typename T>
class QueueAdaptor
{
};

template<>
class QueueAdaptor<MDList>
{
public:
    QueueAdaptor(uint32_t testSize, uint32_t numThread)
        :m_queue(testSize, numThread)
    {}

    void Init()
    {
        m_queue.m_pool.Init();
    }

    void Uninit()
    {
        m_queue.m_pool.Uninit();
    }

    void Insert(uint32_t key)
    {
        m_queue.Insert(key);
    }

    uint32_t DeleteMin()
    {
        return m_queue.DeleteMin();
    }

private:
    MDList m_queue;
};


//-----------------------------------------------------------------------------------
typedef tbb::concurrent_priority_queue<uint32_t, std::greater<uint32_t> > TBBPQ;

template<>
class QueueAdaptor<TBBPQ>
{
public:
    void Init(){}
    void Uninit(){}

    void Insert(uint32_t key)
    {
        m_queue.push(key);
    }

    uint32_t DeleteMin()
    {
        uint32_t val = 0;
        m_queue.try_pop(val);
        return val;
    }

private:
    TBBPQ m_queue;
};


//-----------------------------------------------------------------------------------
typedef std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<uint32_t> > STDPQ;

template<>
class QueueAdaptor<STDPQ>
{
public:
    void Init(){}
    void Uninit(){}

    void Insert(uint32_t key)
    {
        m_mutex.lock();
        m_queue.push(key);
        m_mutex.unlock();
    }

    uint32_t DeleteMin()
    {
        m_mutex.lock();
        uint32_t val = m_queue.top();
        m_queue.pop();
        m_mutex.unlock();
        return val;
    }

private:
    STDPQ m_queue;
    std::mutex m_mutex;
};


//-----------------------------------------------------------------------------------
typedef pq_t LINDENPQ;

template<>
class QueueAdaptor<LINDENPQ>
{
public:
    void Init(){}
    void Uninit(){}

    QueueAdaptor()
    {
        _init_gc_subsystem();
        m_queue = pq_init(32);
    }

    ~QueueAdaptor()
    {
        //pq_destroy(m_queue);
        _destroy_gc_subsystem();
    }

    void Insert(uint32_t key)
    {
        insert(m_queue, key, key);
    }

    uint32_t DeleteMin()
    {
        return deletemin(m_queue);
    }

private:
    pq_t* m_queue;
};

//-----------------------------------------------------------------------------------
typedef set_t SHAVITPQ;

template<>
class QueueAdaptor<SHAVITPQ>
{
public:
    void Init(){}
    void Uninit(){}

    QueueAdaptor()
    {
        _init_set_subsystem();
        m_queue = set_alloc();
    }

    ~QueueAdaptor()
    {
        _destroy_set_subsystem();
    }

    void Insert(uint32_t key)
    {
        set_update(m_queue, key, &key, 0);
    }

    uint32_t DeleteMin()
    {
        return set_deletemin(m_queue);
    }

private:
    SHAVITPQ* m_queue;
};

#endif /* end of include guard: QUEUETESTER_H */
