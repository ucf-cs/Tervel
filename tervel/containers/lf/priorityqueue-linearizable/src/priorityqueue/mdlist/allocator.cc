#include <cassert>
#include <cstring>
#include <priorityqueue/mdlist/allocator.h>

__thread void** Allocator::m_freeNode;
__thread void** Allocator::m_freeDesc;
__thread void** Allocator::m_freeStack;
__thread uint64_t Allocator::m_firstNode;
__thread uint64_t Allocator::m_firstDesc;
__thread uint64_t Allocator::m_firstStack;

Allocator::Allocator(uint64_t cap, uint64_t threadCount, uint64_t nodeSize, uint64_t descSize, uint64_t stackSize)
{
    m_capacity = cap;
    m_threadCount = threadCount;
    m_nodeSize = nodeSize;
    m_descSize = descSize;
    m_stackSize = stackSize;
    m_nodePool = memalign(m_nodeSize, m_nodeSize * m_capacity * m_threadCount);
    m_descPool = memalign(m_descSize, m_descSize * m_capacity * m_threadCount);
    m_stackPool = memalign(m_stackSize, m_stackSize * m_capacity * m_threadCount);

    assert(m_nodePool);
    assert(m_descPool);
    assert(m_stackPool);

    m_ticket = 0;
}

Allocator::~Allocator()
{
    free(m_nodePool);
    free(m_descPool);
    free(m_stackPool);
}

void Allocator::Init()
{
    uint64_t myTicket = m_ticket.fetch_add(1);
    assert(myTicket < m_threadCount);

    m_firstNode = 0;
    m_firstDesc = 0;
    m_firstStack = 0;
    m_freeNode = new void*[m_capacity];
    m_freeDesc = new void*[m_capacity];
    m_freeStack = new void*[m_capacity];

    char* nodeBase = (char*)m_nodePool + myTicket * m_capacity * m_nodeSize;
    char* descBase = (char*)m_descPool + myTicket * m_capacity * m_descSize;
    char* stackBase = (char*)m_stackPool + myTicket * m_capacity * m_stackSize;

    for(uint64_t i = 0; i < m_capacity; ++i)
    {
        m_freeNode[i] = nodeBase + i * m_nodeSize;
        m_freeDesc[i] = descBase + i * m_descSize;
        m_freeStack[i] = stackBase + i * m_stackSize;
    }
}

void Allocator::Uninit()
{
    delete[] m_freeNode;
    delete[] m_freeDesc;
    delete[] m_freeStack;
}
