#ifndef VHEAP_H
#define VHEAP_H

#ifdef __AVX__

#include <stdint.h>
#include <atomic>
#include <mutex>

//32 bytes wide
#ifndef AVX_SIZE
#define AVX_SIZE 32
#endif

class VectorHeap 
{
public:
    typedef uint32_t KeyType;
    static const uint32_t BLOCK_CAPACITY = AVX_SIZE/sizeof(KeyType);
    static const uint32_t BLOCK_COUNT = 1 << 20;
    static const uint32_t HEAP_CAPACITY = BLOCK_CAPACITY * BLOCK_COUNT;

public:
    VectorHeap ();
    ~VectorHeap ();

    void Insert(const KeyType& key);

private:
    void SortBlock(KeyType* block);
    void SortBlockv(KeyType* block);
    bool CompareAndSwapBlockv(KeyType* block, KeyType* parent);

private:
    KeyType* m_key;
    std::mutex* m_lock;

    std::atomic<uint32_t> m_head;
    std::atomic<uint32_t> m_tail;
};

#endif

#endif /* end of include guard: VHEAP_H */
