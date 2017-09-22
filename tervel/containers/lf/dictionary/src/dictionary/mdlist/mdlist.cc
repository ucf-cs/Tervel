//------------------------------------------------------------------------------
// 
//     
//
//------------------------------------------------------------------------------

#include <cstdio>
#include <cstring>
#include <cmath>
#include <immintrin.h>
#include "dictionary/mdlist/mdlist.h"
#include "common/assert.h"

#define SET_ADPINV(_p)    ((Node *)(((uintptr_t)(_p)) | 1))
#define CLR_ADPINV(_p)    ((Node *)(((uintptr_t)(_p)) & ~1))
#define IS_ADPINV(_p)     (((uintptr_t)(_p)) & 1)

#define SET_DELINV(_p)    ((Node *)(((uintptr_t)(_p)) | 2))
#define CLR_DELINV(_p)    ((Node *)(((uintptr_t)(_p)) & ~2))
#define IS_DELINV(_p)     (((uintptr_t)(_p)) & 2)

#define CLR_INVALID(_p)    ((Node *)(((uintptr_t)(_p)) & ~3))
#define IS_INVALID(_p)     (((uintptr_t)(_p)) & 3)

//#define IMB()    __asm__ __volatile__("mfence":::"memory")
//#define IRMB()   __asm__ __volatile__("lfence":::"memory")
//#define IWMB()   __asm__ __volatile__("sfence":::"memory")

ASSERT_CODE(
uint32_t g_count = 0;
uint32_t g_count_ins = 0;
uint32_t g_count_ins_attempt = 0;
uint32_t g_count_del = 0;
uint32_t g_count_del_attempt = 0;
)

//------------------------------------------------------------------------------
//template<int D>
//inline void MDList::KeyToCoord(uint32_t key, uint8_t coord[])
//{
    //const static uint32_t basis[32] = {0xffffffff, 0x10000, 0x800, 0x100, 0x80, 0x40, 0x20, 0x10, 0xC, 0xA, 0x8, 0x7, 0x6, 0x5, 0x5, 0x4,
                                        //0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x2};

    //uint32_t quotient = key;

    //for (int i = D - 1; i >= 0 ; --i) 
    //{
        //coord[i] = quotient % basis[D - 1];
        //quotient /= basis[D - 1];
    //}
//}

template<>
inline void MDList::KeyToCoord<16>(uint32_t key, uint8_t coord[])
{
    static const uint32_t MASK[16] = {0x3u << 30, 0x3u << 28, 0x3u << 26, 0x3u << 24, 0x3u << 22, 0x3u << 20, 0x3u << 18, 0x3u << 16,
                                      0x3u << 14, 0x3u << 12, 0x3u << 10, 0x3u << 8, 0x3u << 6, 0x3u << 4, 0x3u << 2, 0x3u};

    for (uint32_t i = 0; i < 16 ; ++i) 
    {
        coord[i] = ((key & MASK[i]) >> (30 - (i << 1)));
    }
}

//template<int D>
//inline void MDList::KeyToCoord(uint32_t key, uint8_t coord[])
//{
    //uint32_t quotient = key;

    //for (int i = D - 1; i >= 0 ; --i) 
    //{
        //coord[i] = quotient % m_basis;
        //quotient /= m_basis;
    //}
//}


//------------------------------------------------------------------------------
MDList::MDList(uint32_t alloc_cap, uint32_t thread_count, uint32_t keyRange)
    : m_pool(alloc_cap, thread_count, sizeof(Node), sizeof(Desc))
    , m_head(new Node)
    , m_basis(3 + std::ceil(std::pow((float)keyRange, 1.0 / (float)DIMENSION)))
{
    memset(m_head, 0, sizeof(Node));
}

MDList::~MDList()
{
    ASSERT_CODE(
    printf("Total node count %u, Inserts %u/%u, Deletions %u/%u\n", g_count, g_count_ins, g_count_ins_attempt, g_count_del, g_count_del_attempt);
    std::string prefix;
    Traverse(m_head, NULL, 0, prefix);
    )
}


//------------------------------------------------------------------------------
void MDList::Insert(uint32_t key)
{
    ASSERT_CODE(__sync_fetch_and_add(&g_count_ins_attempt, 1);)

    //Allocate new node
    //Decompose Key into mutli-dimension coordinates
    Node* new_node = (Node*)m_pool.AllocNode();
    new_node->m_key = key;
    KeyToCoord<DIMENSION>(key, new_node->m_coord);

    Node* pred = NULL;      //pred node
    Node* curr = m_head;    //curr node
    uint32_t dim = 0;       //the dimension of curr node
    uint32_t pred_dim = 0;  //the dimesnion of pred node

    while(true)
    {
        LocatePred(new_node->m_coord, pred, curr, dim, pred_dim);

        Node* pred_child = pred->m_child[pred_dim];

        //We are not updating existing node, remove this line to allow update
        if(dim == DIMENSION && !IS_DELINV(pred_child))
        {
            return; 
        }

        //There are three possible inserting positions:
        //1. on link edge from pred to curr node
        //   [repalce pointer in pred with new_node and make curr the child of new_node] 
        //
        //2. before curr node
        //   [replace pointer in pred with new_node and copy all children up to dim to new_node] 
        //
        //3. after pred node
        //   [curr node must be NULL, no need to pend task]
        //
        //4. override the current node
        //   [insert new_node and disconnet curr node]

        //Atomically update pred node's child pointer
        //if cas fails, it means another node has succesfully inserted inbetween pred and curr
        Node* expected = curr;
        if(IS_DELINV(pred_child))
        {
            expected = SET_DELINV(curr); 
            //if child adoption is need, we take the chance to do physical deletion
            //Otherwise, we CANNOT force full scale adoption
            if(dim == DIMENSION - 1)
            {
                dim = DIMENSION;
            }
        }

        if(pred_child == expected)
        {
            Desc* desc = FillNewNode(new_node, pred, expected, dim, pred_dim);

            pred_child = __sync_val_compare_and_swap(&pred->m_child[pred_dim], expected, new_node);

            if(pred_child == expected)
            {
                //The insertion succeeds, complete remaining pending task for this node if any
                if(desc) 
                {
                    //Case 2 inserting new_node at the same dimesion as pred node
                    //and push the curr node down one dimension
                    //We need to help curr node finish the pending task 
                    //because the children adoption in new_node might need the adoption from curr node
                    Desc* pending = curr ? curr->m_pending : NULL;
                    if(pending)
                    {
                        FinishInserting(curr, pending);
                    }

                    FinishInserting(new_node, desc);
                }

                ASSERT_CODE(
                        if(dim != DIMENSION){
                        __sync_fetch_and_add(&g_count, 1);
                        __sync_fetch_and_add(&g_count_ins, 1);}
                        //printf("[%u] Inserting node %p key %u as %dth child of node %p key %u\n", g_count_ins_attempt, new_node, new_node->m_key, pred_dim, pred, pred ? pred->m_key : 0);
                        )
                break;
            }
        }

        //If the code reaches here it means the CAS failed
        //Three reasons why CAS may fail:
        //1. the child slot has been marked as invalid by parents
        //2. another thread inserted a child into the slot
        //3. another thread deleted the child
        if(IS_ADPINV(pred_child))
        {
            pred = NULL;
            curr = m_head;
            dim = 0;
            pred_dim = 0;
        }
        else if(CLR_INVALID(pred_child) != curr)
        {
            curr = pred;
            dim = pred_dim;
        }
        else
        {
            //Do nothing, no need to find new inserting poistion
            //retry insertion at the same location
        }

        //If pending taks is allocate free it now
        //it might not be needed in the next try
        //No need to restore other fields as they will be initilized in the next iteration 
        if(new_node->m_pending)
        {
            m_pool.FreeDesc(new_node->m_pending);
            new_node->m_pending = NULL;
        }
    }
}


inline void MDList::LocatePred(uint8_t coord[], Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim)
{
    //Locate the proper position to insert
    //traverse list from low dim to high dim
    while(dim < DIMENSION)
    {
        //Loacate predecessor and successor
        while(curr && coord[dim] > curr->m_coord[dim])
        {
            pred_dim = dim;
            pred = curr;

            Desc* pending = curr->m_pending;
            if(pending && dim >= pending->pred_dim && dim <= pending->dim)
            {
                FinishInserting(curr, pending);

            }
            curr = CLR_INVALID(curr->m_child[dim]);
        }

        //no successor has greater coord at this dimension
        //the position after pred is the insertion position
        if(curr == NULL || coord[dim] < curr->m_coord[dim]) 
        {
            //done searching
            break;
        }
        //continue to search in the next dimension 
        //if coord[dim] of new_node overlaps with that of curr node
        else
        {
            //dim only increases if two coords are exactly the same
            ++dim;
        }
    }
}


inline MDList::Desc* MDList::FillNewNode(Node* new_node, Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim)
{
    Desc* desc = NULL;
    if(pred_dim != dim)
    {
        //descriptor to instruct other insertion task to help migrate the children
        desc = (Desc*)m_pool.AllocDesc();
        desc->curr = CLR_DELINV(curr);
        desc->pred_dim = pred_dim;
        desc->dim = dim;
    }

    //Fill values for new_node, m_child is set to 1 for all children before pred_dim
    //pred_dim is the dimension where new_node is inserted, all dimension before that are invalid for new_node
    for(uint32_t i = 0; i < pred_dim; ++i)
    {
        new_node->m_child[i] = (Node*)0x1;
    }
    //be careful with the length of memset, should be DIMENSION - pred_dim NOT (DIMENSION - 1 - pred_dim)
    memset(new_node->m_child + pred_dim, 0, sizeof(Node*) * (DIMENSION - pred_dim));
    if(dim < DIMENSION)
    {
        //If curr is marked for deletion or overriden, we donnot link it. 
        //Instead, we adopt ALL of its children
        new_node->m_child[dim] = curr;
    }
    new_node->m_pending = desc;

    return desc;
}


inline void MDList::FinishInserting(Node* n, Desc* desc)
{
    uint32_t pred_dim = desc->pred_dim;    
    uint32_t dim = desc->dim;    
    Node* curr = desc->curr;

    for (uint32_t i = pred_dim; i < dim; ++i) 
    {
        Node* child = curr->m_child[i];

        //Children slot of curr_node need to be marked as invalid 
        //before we copy them to new_node
        //while(!IS_ADPINV(child) && !__sync_bool_compare_and_swap(&curr->m_child[i], child, SET_ADPINV(child)))
        //{
            //child = curr->m_child[i];
        //}
        child = __sync_fetch_and_or(&curr->m_child[i], 0x1);
        child = CLR_ADPINV(curr->m_child[i]);
        if(child)
        {
            //Adopt children from curr_node's
            if(n->m_child[i] == NULL)
            {
                __sync_bool_compare_and_swap(&n->m_child[i], NULL, child);
            }
        }
    }

    //Clear the pending task
    if(n->m_pending == desc && __sync_bool_compare_and_swap(&n->m_pending, desc, NULL))
    {
        //TODO:recycle desc
    }
}


//------------------------------------------------------------------------------
bool MDList::Delete(uint32_t key)
{
    ASSERT_CODE(__sync_fetch_and_add(&g_count_del_attempt, 1);)

    uint8_t coord[DIMENSION];
    KeyToCoord<DIMENSION>(key, coord);
    Node* pred = NULL;      //pred node
    Node* curr = m_head;    //curr node
    uint32_t dim = 0;       //the dimension of curr node
    uint32_t pred_dim = 0;  //the dimesnion of pred node

    while(true)
    {
        LocatePred(coord, pred, curr, dim, pred_dim);

        if(dim == DIMENSION)
        {
            Node* pred_child = pred->m_child[pred_dim];

            if(pred_child == curr)
            {
                ASSERT(curr, "node must exisit");
                ASSERT(key == curr->m_key, "asked to insert as sibling but keys are unequal");

                pred_child = __sync_val_compare_and_swap(&pred->m_child[pred_dim], curr, SET_DELINV(curr));
            }

            //1. successfully marked node for deletion
            if(pred_child == curr)
            {
                ASSERT_CODE(__sync_fetch_and_add(&g_count, -1);)
                ASSERT_CODE(__sync_fetch_and_add(&g_count_del, 1);)
                ASSERT(curr->m_key == key, "marked wrong node for deletion");
                //printf("delete key %u\n", key);
                return true;
            }
            //2. Node is marked for deletion by another thread 
            else if(IS_DELINV(pred_child) && CLR_INVALID(pred_child) == curr)
            {
                return false;
            }
            //3. pred_child has changed or been adopted, start over
            else
            {
                pred = NULL;
                curr = m_head;
                dim = 0;
                pred_dim = 0;
            }
        }
        //Node does not exist
        else
        {
            return false;
        }
    }
}


//------------------------------------------------------------------------------
bool MDList::Find(uint32_t key)
{
    //TODO: may be use specilized locatedPred to speedup
    uint8_t coord[DIMENSION];
    KeyToCoord<DIMENSION>(key, coord);
    Node* pred = NULL;      //pred node
    Node* curr = m_head;    //curr node
    uint32_t dim = 0;       //the dimension of curr node
    uint32_t pred_dim = 0;  //the dimesnion of pred node

    LocatePred(coord, pred, curr, dim, pred_dim);

    return dim == DIMENSION;
}


//------------------------------------------------------------------------------
inline void MDList::Traverse(Node* n, Node* parent, int dim, std::string& prefix)
{
    if(!IS_DELINV(n))
    {
        printf("%s", prefix.c_str());
        printf("Node [%p] Key [%u] DIM [%d] of Parent[%p]\n", n, n->m_key, dim, parent);
    }
    n = CLR_DELINV(n);    

    //traverse from last dimension up to current dim
    //The valid children include child nodes up to dim
    //e.g. a node on dimension 3 has only valid children on dimensions 3~8
    for (int i = DIMENSION - 1; i >= dim; --i) 
    {
        Node* child = n->m_child[i];
        ASSERT(!IS_ADPINV(child), "corrupt mdlist, invalid child in a valid dimension");

        if(child != NULL)
        {
            prefix.push_back('|');
            prefix.insert(prefix.size(), i, ' ');

            Traverse(child, n, i, prefix);

            prefix.erase(prefix.size() - i - 1, i + 1);
        }
    }
}
