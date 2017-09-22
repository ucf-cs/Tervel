#ifndef MONOHEAP_H
#define MONOHEAP_H

#ifdef __AVX__

#include <stdint.h>

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 128
#endif

#ifndef AVX_SIZE
#define AVX_SIZE 256
#endif

class MonoHeap 
{
public:
    typedef uint32_t KeyType;
    static const uint32_t BLOCK_CAPACITY = AVX_SIZE/sizeof(KeyType);

private:
    //A single block of array based heap (or sorted array)
    struct Block
    {
        Block();

        KeyType m_key[BLOCK_CAPACITY]; 
        Block* m_layers[BLOCK_CAPACITY];
        Block* m_prev;
        Block* m_next;
        uint32_t m_size;
    };

public:
    MonoHeap ();
    ~MonoHeap ();

    void Insert(const KeyType& key);

private:
    uint32_t ComputeIndex(const KeyType& key, const Block* b);
    uint32_t ComputeIndex1(const KeyType& key, const Block* b);

    void DeleteBlock(Block* b);

private:
    Block* m_root;
    Block* m_reverse;
    uint32_t m_blockCount;
};

#endif //AVX

#endif /* end of include guard: MONOHEAP_H */
