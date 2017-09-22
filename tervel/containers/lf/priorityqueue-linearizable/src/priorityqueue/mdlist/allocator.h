#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <cstdint>
#include <malloc.h>
#include <atomic>
#include <common/assert.h>

class Allocator 
{
public:
    Allocator (uint64_t cap, uint64_t threadCount, uint64_t nodeSize, uint64_t descSize, uint64_t stackSize);
    ~Allocator ();

    //Every thread need to call init once before any allocation
    void Init();
    void Uninit();

    void* AllocNode()
    {
        ASSERT(m_firstNode < m_capacity, "out of capacity");
        void* ret = m_freeNode[m_firstNode++];

        return ret;
    }

    void FreeNode(void* p)
    {
        //assert(p);
        m_freeNode[--m_firstNode] = p;
    }

    void* AllocDesc()
    {
        ASSERT(m_firstDesc < m_capacity, "out of capacity");
        void* ret = m_freeDesc[m_firstDesc++];

        return ret;
    }

    void FreeDesc(void* p)
    {
        //assert(p);
        m_freeDesc[--m_firstDesc] = p;
    }

    void* AllocStack()
    {
        ASSERT(m_firstStack < m_capacity, "out of capacity");
        void* ret = m_freeStack[m_firstStack++];

        return ret;
    }

    void FreeStack(void* p)
    {
        //assert(p);
        m_freeStack[--m_firstStack] = p;
    }
    
private:
    void* m_nodePool;
    void* m_descPool;
    void* m_stackPool;
    uint64_t m_capacity;      //number of elements T in the pool
    uint64_t m_nodeSize;
    uint64_t m_descSize;
    uint64_t m_stackSize;
    uint64_t m_threadCount;
    std::atomic<uint64_t> m_ticket;

    static __thread void** m_freeNode;
    static __thread void** m_freeDesc;
    static __thread void** m_freeStack;
    static __thread uint64_t m_firstNode;
    static __thread uint64_t m_firstDesc;
    static __thread uint64_t m_firstStack;
};

#endif /* end of include guard: ALLOCATOR_H */
