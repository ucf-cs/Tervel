//------------------------------------------------------------------------------
// 
//     
//
//------------------------------------------------------------------------------

#ifdef __AVX__

#include <cstdio>
#include <cstring>
#include <malloc.h>
#include <immintrin.h>
#include "priorityqueue/vectorheap/vheap.h"

inline void print_vector_256(const __m256& val)
{
    uint32_t p[8];
    _mm256_storeu_ps((float*)p, val);
    printf("[%X %X %X %X %X %X %X %X]", p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
}

inline void print_vector_128(const __m128& val)
{
    uint32_t p[4];
    _mm_store_ps((float*)p, val);
    printf("[%X %X %X %X]", p[0],p[1],p[2],p[3]);
}

VectorHeap::VectorHeap()
{
    m_key = (KeyType*)memalign(32u, sizeof(KeyType) * HEAP_CAPACITY);
    //m_key = new KeyType[HEAP_CAPACITY];
    m_lock = new std::mutex[BLOCK_COUNT];
    m_head = 0;
    m_tail = 0;
}


VectorHeap::~VectorHeap()
{
    //for (uint32_t i = 0; i < m_tail.load() / BLOCK_CAPACITY; i += BLOCK_CAPACITY) 
    //{
        //printf("[%d %d %d %d %d %d %d %d]\n", m_key[i + 0],m_key[i + 1],m_key[i + 2],m_key[i + 3],m_key[i + 4],m_key[i + 5],m_key[i + 6],m_key[i + 7]);
    //}

    free(m_key);
    delete[] m_lock;
}

void VectorHeap::Insert(const KeyType& key)
{
    uint32_t pos = m_tail.fetch_add(1);

    //printf("Key %d, Position %d ", key, pos);

    m_key[pos] = key;

    if((pos + 1) % BLOCK_CAPACITY == 0)
    {
        uint32_t  block_pos = pos / BLOCK_CAPACITY;

        m_lock[block_pos].lock();

        //Sort within block
        SortBlock(&m_key[block_pos * BLOCK_CAPACITY]);

        //Heapfy the new block by recurisvely compare it with its parents
        bool has_any_swap = true;
        uint32_t parent_pos = 0;

        while (block_pos > 0 && has_any_swap)
        {
            parent_pos = (block_pos - 1) >> 1;

            m_lock[parent_pos].lock();

            has_any_swap = CompareAndSwapBlockv(&m_key[block_pos * BLOCK_CAPACITY], &m_key[parent_pos * BLOCK_CAPACITY]);

            m_lock[block_pos].unlock();

            block_pos = parent_pos;
        }

        m_lock[block_pos].unlock();
    }

    //printf("\n");
}

inline bool VectorHeap::CompareAndSwapBlockv(KeyType* block, KeyType* parent)
{
    __m256 b = _mm256_load_ps((float*)block);
    __m256 p = _mm256_load_ps((float*)parent);

    __m256 cmp = _mm256_cmp_ps(b, p, _CMP_LT_OS);

    __m256 new_p = _mm256_blendv_ps(p, b, cmp);
    __m256 new_b = _mm256_blendv_ps(b, p, cmp);

    _mm256_store_ps((float*)block, new_b);
    _mm256_store_ps((float*)parent, new_p);

    return !_mm256_testz_ps(cmp, cmp);
}

inline void VectorHeap::SortBlockv(KeyType* block)
{
    //Load keys within one block
    __m256 b = _mm256_load_ps((float*)block);
    //__m128 bl = _mm_load_ps((float*)block);
    //__m128 bh = _mm_load_ps((float*)(block + 4));

    __m256 b1 = _mm256_movehdup_ps(b);
    __m256 b2 = _mm256_moveldup_ps(b);

    ////Stage 1
    ////Compare two registers
    //__m128 cmp = _mm_cmplt_ss(bl, bh);

    ////Swap keys if cmpare is sucessful
    ////Blend bl and bh with the mask cmp and ~cmp
    //__m128 b = _mm_blendv_ps(bl, bh, cmp);
    //__m128 b2 = _mm_blendv_ps(bh, bl, cmp);

    print_vector_256(b1);
    //print_vector_128(bl);
    //print_vector_128(bh);
}

inline void VectorHeap::SortBlock(KeyType* block)
{
#define MAX(x, y) (x ^ ((x ^ y) & -(x < y)))
#define MIN(x, y) (y ^ ((x ^ y) & -(x < y)))
#define CMP_SWAP(a, i, j){int temp = MAX(a[i], a[j]); a[j] = MIN(a[i], a[j]); a[i] = temp;}

    CMP_SWAP(block, 0, 1); CMP_SWAP(block, 2, 3); CMP_SWAP(block, 4, 5); CMP_SWAP(block, 6, 7);
    CMP_SWAP(block, 0, 2); CMP_SWAP(block, 1, 3); CMP_SWAP(block, 4, 6); CMP_SWAP(block, 5, 7);
    CMP_SWAP(block, 1, 2); CMP_SWAP(block, 5, 6); CMP_SWAP(block, 0, 4); CMP_SWAP(block, 1, 5);
    CMP_SWAP(block, 2, 6); CMP_SWAP(block, 3, 7); CMP_SWAP(block, 2, 4); CMP_SWAP(block, 3, 5);
    CMP_SWAP(block, 1, 2); CMP_SWAP(block, 3, 4); CMP_SWAP(block, 5, 6);
}

#endif
