#include <cassert>
#include <cstring>
#include "dictionary/mdlist/allocator.h"

__thread void** Allocator::m_freeNode;
__thread void** Allocator::m_freeDesc;
__thread uint64_t Allocator::m_firstNode;
__thread uint64_t Allocator::m_firstDesc;

Allocator::Allocator(uint64_t cap, uint64_t threadCount, uint64_t nodeSize, uint64_t descSize)
{
    m_capacity = cap;
    m_threadCount = threadCount;
    m_nodeSize = nodeSize;
    m_descSize = descSize;
    m_nodePool = memalign(m_nodeSize, m_nodeSize * m_capacity * m_threadCount);
    m_descPool = memalign(m_descSize, m_descSize * m_capacity * m_threadCount);

    assert(m_nodePool);
    assert(m_descPool);

    m_ticket = 0;
}

Allocator::~Allocator()
{
    free(m_nodePool);
    free(m_descPool);
}

void Allocator::Init(uint32_t threadId)
{
    assert(threadId < m_threadCount);

    m_firstNode = 0;
    m_firstDesc = 0;
    m_freeNode = new void*[m_capacity];
    m_freeDesc = new void*[m_capacity];

    char* nodeBase = (char*)m_nodePool + threadId * m_capacity * m_nodeSize;
    char* descBase = (char*)m_descPool + threadId * m_capacity * m_descSize;

    for(uint64_t i = 0; i < m_capacity; ++i)
    {
        m_freeNode[i] = nodeBase + i * m_nodeSize;
        m_freeDesc[i] = descBase + i * m_descSize;
    }
}

void Allocator::Uninit()
{
    delete[] m_freeNode;
    delete[] m_freeDesc;
}
