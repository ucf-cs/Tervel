//------------------------------------------------------------------------------
// 
//     
//
//------------------------------------------------------------------------------

#ifdef __AVX__

#include <cstdio>
#include <cstring>
#include <immintrin.h>
#include "priorityqueue/monoheap/monoheap.h"

inline int Log2 (int x)
{
    //Returns the number of trailing 0-bits in x, 
    //starting at the least significant bit position. 
    //If x is 0, the result is undefined.
    return __builtin_ctz (x);
}

inline void print_vector(const __m256& val)
{
    const int* p = reinterpret_cast<const int*>(&val);
    printf("[%x %x %x %x %x %x %x %x]\n", p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
}

MonoHeap::Block::Block()
{
    std::memset(m_key, ~0, BLOCK_CAPACITY);
    std::memset(m_layers, 0, BLOCK_CAPACITY);
    m_prev = 0;
    m_next = 0;
    m_size = 0;
}

MonoHeap::MonoHeap()
{
    m_root = new Block;
    m_reverse = m_root;
    m_blockCount = 0;

    m_root->m_key[0] = 5;
    m_root->m_key[1] = 50;
    m_root->m_key[2] = 500;
    m_root->m_key[3] = 5000;
    m_root->m_key[4] = 50000;
    m_root->m_key[5] = 500000;
    m_root->m_key[6] = 5000000;
    m_root->m_key[7] = 50000000;
}

MonoHeap::~MonoHeap()
{
   //printf("MonoHeap destruction: ");
   //DeleteBlock(m_root);
   //printf("\nDeleted %d blocks\n", m_blockCount);
}

void MonoHeap::Insert(const KeyType& key)
{
    ComputeIndex1(key, m_root);

    //Block* cur_block = m_reverse;
    //uint32_t ins_index = 0;

    ////Search for an insertion position at the end of current block
    //while(ins_index < cur_block->m_size)
    //{
        //ins_index = ComputeIndex(key, cur_block);

        //if(ins_index < cur_block->m_size)
        //{
            ////Step on to next layer 
            //if(cur_block->m_layers[ins_index] != NULL)
            //{
                //cur_block = cur_block->m_layers[ins_index]; 
                //ins_index = 0;
            //}
            ////Next layer is empty, create one
            //else
            //{
                ////fill new block with desired key
                //Block* new_block = new Block;
                //new_block->m_key[new_block->m_size] = key;
                //new_block->m_size++;

                ////use CAS here
                //cur_block->m_layers[ins_index] = new_block;
            //}
        //}
    //}

    //if(ins_index == cur_block->m_size)
    //{
        ////Need to use CAS here
        //cur_block->m_size++;
        //cur_block->m_key[ins_index] = key;

        ////Need to update val
    //}
    //else if(ins_index > BLOCK_CAPACITY)
    //{
        ////Append new block at the end
        //Block* new_block = new Block;
        //new_block->m_key[new_block->m_size] = key;
        //new_block->m_size++;

        ////use CAS here
        //cur_block->m_layers[ins_index] = new_block;
    //}
}

uint32_t MonoHeap::ComputeIndex(const KeyType& key, const Block* b)
{
    //printf("key %d ", key);

    //Populate avx register with duplicates of key
    __m256 keyv = _mm256_broadcast_ss((float*)&key);

    //Load current key
    __m256 oldKeyv = _mm256_load_ps((float*)b->m_key);

    //Set output vector to true if keyv is GREATER OR EQUAL to oldKeyv
    //Since the latter instruction load cmp_mask from low address to high address
    //This gives the correct indexs for key insertion in ascending order
    __m256 cmp = _mm256_cmp_ps(keyv, oldKeyv, _CMP_GE_OS);
    int cmp_mask = _mm256_movemask_ps(cmp);

    //printf("mask %08x\n", cmp_mask);

    return Log2(++cmp_mask);
}

uint32_t MonoHeap::ComputeIndex1(const KeyType& key, const Block* b)
{
    uint32_t i;
    for(i = 7; i >= 0; --i)
    {
        if(key < b->m_key[i])
            break;
    }
    return i;
}


void MonoHeap::DeleteBlock(Block* b)
{
    m_blockCount++;

    for (uint32_t i = 0; i < b->m_size; i++) 
    {
        if(b->m_layers[i])
        {
            DeleteBlock(b->m_layers[i]);
        }
        
        printf("%d,", b->m_key[i]);
    }

    delete b;
}

#endif //avx
