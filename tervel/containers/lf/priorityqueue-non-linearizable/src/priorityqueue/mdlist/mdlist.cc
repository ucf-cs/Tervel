//------------------------------------------------------------------------------
// 
//     
//
//------------------------------------------------------------------------------

#include <cstdio>
#include <cstring>
#include <immintrin.h>
#include "priorityqueue/mdlist/mdlist.h"
#include "assert.h"

#define SET_ADPINV(_p)    ((Node *)(((uintptr_t)(_p)) | 1))
#define CLR_ADPINV(_p)    ((Node *)(((uintptr_t)(_p)) & ~1))
#define IS_ADPINV(_p)     (((uintptr_t)(_p)) & 1)

#define SET_PRGINV(_p)    ((Node *)(((uintptr_t)(_p)) | 2))
#define CLR_PRGINV(_p)    ((Node *)(((uintptr_t)(_p)) & ~2))
#define IS_PRGINV(_p)     (((uintptr_t)(_p)) & 2)

#define CLR_INVALID(_p)    ((Node *)(((uintptr_t)(_p)) & ~3))
#define IS_INVALID(_p)     (((uintptr_t)(_p)) & 3)

#define SET_MARKED(_p)    ((Stack *)(((uintptr_t)(_p)) | 1))
#define CLR_MARKED(_p)    ((Stack *)(((uintptr_t)(_p)) & ~1))
#define IS_MARKED(_p)     (((uintptr_t)(_p)) & 1)

#define SET_DELETED(_p)    ((Node *)(((uintptr_t)(_p)) | 1))
#define CLR_DELETED(_p)    ((Node *)(((uintptr_t)(_p)) & ~1))
#define IS_DELETED(_p)     (((uintptr_t)(_p)) & 1)

ASSERT_CODE(
uint32_t g_count = 0;
uint32_t g_count_ins = 0;
uint32_t g_count_ins_attempt = 0;
uint32_t g_count_ins_dup = 0;
uint32_t g_count_rewind_neq = 0;
uint32_t g_count_del = 0;
uint32_t g_count_del_attempt = 0;
uint32_t g_count_maxrepath = 0;
uint32_t g_count_avgrepath = 0;
uint32_t g_count_avgrenode = 0;
uint32_t g_count_avgnullnode = 0;
uint32_t g_count_purge = 0;
)

//------------------------------------------------------------------------------
template<int D>
inline void MDList::KeyToCoord(uint32_t key, uint8_t coord[])
{
    const static uint32_t basis[32] = {0xffffffff, 0x10000, 0x800, 0x100, 0x80, 0x40, 0x20, 0x10, 0xC, 0xA, 0x8, 0x7, 0x6, 0x5, 0x5, 0x4,
                                        0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x2};

    uint32_t quotient = key;

    for (int i = D - 1; i >= 0 ; --i) 
    {
        coord[i] = quotient % basis[D - 1];
        quotient /= basis[D - 1];
    }
}


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

//------------------------------------------------------------------------------
MDList::MDList(uint32_t alloc_cap, uint32_t thread_count)
    : m_pool(alloc_cap, thread_count + 1, sizeof(Node), sizeof(Desc), sizeof(Stack))
    , m_head(new Node)
    , m_stack(new Stack)
    , m_purge(NULL)
{
    memset(m_head, 0, sizeof(Node));
    m_head->m_purged = SET_DELETED(NULL);
    m_stack->m_head = m_head;
    for(uint32_t i = 0; i < DIMENSION; ++i)
    {
        m_stack->m_del[i] = m_head; 
    }
}

MDList::~MDList()
{
    ASSERT_CODE(
    printf("Total node count %u, Inserts %u/%u/%u, Rewind %u, Deletions %u/%u, Purge %u, Repath Node/Null/Avg/Max [%u/%u/%u/%u]\n", g_count, g_count_ins, g_count_ins_dup, g_count_ins_attempt, g_count_rewind_neq, g_count_del, g_count_del_attempt,  g_count_purge, g_count_avgrenode/g_count_del_attempt, g_count_avgnullnode/g_count_del_attempt, g_count_avgrepath/g_count_del_attempt, g_count_maxrepath);
    //std::string prefix;
    //Traverse(m_head, NULL, 0, prefix);
    //prefix.clear();
    //if(m_stack->m_head != m_head) Traverse(m_stack->m_head, NULL, 0, prefix);
    );
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
    new_node->m_purged = NULL;

    Stack path;
    Node* pred = NULL;      //pred node
    Node* curr = m_head;    //curr node
    uint32_t dim = 0;       //the dimension of curr node
    uint32_t pred_dim = 0;  //the dimesnion of pred node
    path.m_head = curr;

    while(true)
    {
        LocatePred(&path, new_node, pred, curr, dim, pred_dim);

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
        //4. as the sibling of curr node
        //   [insert new_node to sibling list of curr node]


        //Case 1,  2 and 3, insertion after an exsiting node
        //use pending task to guide other thread help finishing the children adoption 
        if(dim != DIMENSION)
        {
            //if this insertion of new_node as child[pred_dim] of pred node happens to 
            //fall within the range of [pred_dim, dim] of pending children migration of pred node
            //Desc* pending = pred ? pred->m_pending : NULL;
            Desc* pending = pred->m_pending;
            if(pending && pred_dim >= pending->pred_dim && pred_dim <= pending->dim)
            {
                FinishInserting(pred, pending);

                //re-compute insertion poistion, but only start from pred
                //no need to start from head because the interupting insertion happens after pred
                curr = pred;
                dim = pred_dim;

                continue;
            }

            //Case 2 inserting new_node at the same dimesion as pred node
            //and push the curr node down one dimension
            //We need to help curr node finish the pending task 
            //because the children adoption in new_node might need the adoption from curr node
            pending = curr ? curr->m_pending : NULL;
            if(pending && pred_dim != dim)
            {
                //We can only see curr's pending if curr can be reached in list
                //No need to restart because curr's pending only affect curr's children
                FinishInserting(curr, pending);
            }

            //Atomically update pred node's child pointer
            //if cas fails, it means another node has succesfully inserted inbetween pred and curr
            //Node** insert_addr = pred ? &pred->m_child[pred_dim] : &m_head;
            Node** insert_addr = &pred->m_child[pred_dim];
            Node* pred_child = *insert_addr;

            if(pred_child == curr)
            {
                Desc* desc = FillNewNode(new_node, pred, curr, dim, pred_dim);

                pred_child = __sync_val_compare_and_swap(insert_addr, curr, new_node);

                if(pred_child == curr)
                {
                    //The insertion succeeds, complete remaining pending task for this node if any
                    if(desc)
                    {
                        FinishInserting(new_node, desc);
                    }

                    if(IS_DELETED(pred->m_purged) && !IS_DELETED(new_node->m_purged))
                    {
                        Stack* _s = __sync_fetch_and_or(&m_stack, 0x1);
                        //Stack* _s = m_stack;
                        Stack* s = CLR_MARKED(_s);
                        Stack* ex_s = NULL;
                        Stack* new_s = (Stack*)m_pool.AllocStack();

                        do
                        {
                            if(s->m_head == path.m_head)
                            {
                                if(key <= s->m_del[DIMENSION - 1]->m_key)
                                {
                                    //rewind stack to new_node
                                    new_s->m_head = path.m_head;
                                    for(uint32_t i = 0; i < pred_dim; ++i) new_s->m_del[i] = path.m_del[i]; 
                                    for(uint32_t i = pred_dim; i < DIMENSION; ++i) new_s->m_del[i] = pred;
                                }
                                else if(ex_s == NULL && IS_MARKED(_s))
                                {
                                    ASSERT_CODE( __sync_fetch_and_add(&g_count_rewind_neq, 1);)
                                    *new_s = *s; 
                                }
                                else
                                {
                                    break;
                                }
                            }
                            else if(s->m_head->m_seq > path.m_head->m_seq)
                            {
                                Node* purged = CLR_DELETED(path.m_head->m_purged);
                                if(purged->m_key <= key)
                                {
                                    //rewind stack to purged 
                                    new_s->m_head = CLR_DELETED(purged->m_purged);
                                    for(uint32_t i = 0; i < DIMENSION; ++i) new_s->m_del[i] = new_s->m_head; 
                                }
                                else
                                {
                                    //rewind stack to new_node 
                                    new_s->m_head = path.m_head;
                                    for(uint32_t i = 0; i < pred_dim; ++i) new_s->m_del[i] = path.m_del[i]; 
                                    for(uint32_t i = pred_dim; i < DIMENSION; ++i) new_s->m_del[i] = pred;
                                }
                            }
                            else
                            {
                                ASSERT(s->m_head->m_seq < path.m_head->m_seq, "stack head should be older than path head");

                                Node* purged = CLR_DELETED(s->m_head->m_purged);
                                if(purged->m_key <= s->m_del[DIMENSION - 1]->m_key)
                                {
                                    //rewind stack purged 
                                    new_s->m_head = CLR_DELETED(purged->m_purged);
                                    for(uint32_t i = 0; i < DIMENSION; ++i) new_s->m_del[i] = new_s->m_head; 
                                }
                                //TODO: poor scalability, it's strage because the rewind caused by the following lines is not much
                                //Maybe it will cause a lot of repathing
                                //else if(ex_s == NULL && IS_MARKED(_s))
                                //{
                                    //ASSERT_CODE( __sync_fetch_and_add(&g_count_rewind_neq, 1);)
                                    //*new_s = *s; 
                                //}
                                else
                                {
                                    break; 
                                }
                            }

                            ex_s = _s;
                            _s = __sync_val_compare_and_swap(&m_stack, ex_s, new_s);
                            s = CLR_MARKED(_s);
                        }
                        while(_s != ex_s && !IS_DELETED(new_node->m_purged));
                    }

                    ASSERT_CODE(
                            __sync_fetch_and_add(&g_count, 1);
                            __sync_fetch_and_add(&g_count_ins, 1);
                            //printf("[%u] Inserting node %p key %u as %dth child of node %p key %u\n", g_count_ins_attempt, new_node, new_node->m_key, pred_dim, pred, pred ? pred->m_key : 0);
                            )
                    break;
                }
            }

            //If the code reaches here it means the CAS failed
            //Three reasons why CAS may fail:
            //1. the child slot has been marked as invalid by parents
            //2. another thread inserted the child into the slot
            if(IS_INVALID(pred_child))
            {
                curr = m_head;
                dim = 0;
                pred = NULL;
                pred_dim = 0;
                path.m_head = curr;
            }
            else
            {
                curr = pred;
                dim = pred_dim;
            }

            //If pending taks is allocate free it now
            //it might not be needed in the next try
            //No need to restore other fields as they will be initilized in the next iteration 
            //if(new_node->m_pending)
            //{
                //m_pool.FreeDesc(new_node->m_pending);
                //new_node->m_pending = NULL;
            //}
        }
        else
        {
            ASSERT_CODE( if(new_node != curr) { __sync_fetch_and_add(&g_count_ins_dup, 1); })
            ASSERT(new_node->m_key == curr->m_key, "asked to insert as sibling but keys are unequal");
            //Case 4. The only case where dim == DIMENSION 
            //is when the key of new_node is equal to that of curr
            //This means that the search has exhausted all dimensions 
            //and not been able to find inserting position of case 1&2

            break;
        }
    }
}


inline void MDList::LocatePred(Stack* path, Node* new_node, Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim)
{
    //Locate the proper position to insert
    //traverse list from low dim to high dim
    while(dim < DIMENSION)
    {
        //Loacate predecessor and successor
        while(curr && new_node->m_coord[dim] > curr->m_coord[dim])
        {
            pred_dim = dim;
            pred = curr;
            curr = CLR_INVALID(curr->m_child[dim]);
        }

        //no successor has greater coord at this dimension
        //the position after pred is the insertion position
        if(curr == NULL || new_node->m_coord[dim] < curr->m_coord[dim]) 
        {
            //done searching
            break;
        }
        //continue to search in the next dimension 
        //if coord[dim] of new_node overlaps with that of curr node
        else
        {
            path->m_del[dim] = curr;

            //dim only increases if two coords are exactly the same
            ++dim;
        }
    }

    //ASSERT(path->m_del[0], "wrong path");
}



inline MDList::Desc* MDList::FillNewNode(Node* new_node, Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim)
{
    Desc* desc = NULL;
    if(pred_dim != dim)
    {
        //descriptor to instruct other insertion task to help migrate the children
        desc = (Desc*)m_pool.AllocDesc();
        desc->curr = curr;
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
    new_node->m_child[dim] = curr;
    new_node->m_pending = desc; 

    return desc;
}


inline void MDList::FinishInserting(Node* n, Desc* desc)
{
    uint32_t pred_dim = desc->pred_dim;    
    uint32_t dim = desc->dim;    
    Node* curr = desc->curr;

    char inv_flag = 0x1;

    for (uint32_t i = pred_dim; i < dim; ++i) 
    {
        Node* child = curr->m_child[i];

        //Children slot of curr_node need to be marked as invalid 
        //before we copy them to new_node
        //while(!IS_ADPINV(child) && !__sync_bool_compare_and_swap(&curr->m_child[i], child, SET_ADPINV(child)))
        while(!IS_PRGINV(child) && !IS_ADPINV(child) && !__sync_bool_compare_and_swap(&curr->m_child[i], child, (uintptr_t)child | inv_flag))
        {
            child = curr->m_child[i];
        }

        if(IS_PRGINV(child)) inv_flag = 0x2;
        
        //TODO:Skip the following migration process is a purge is in progress
        //if(IS_PRGINV(child)) break;
        //Have to re-read, because the above while loop may exit due to a CAS success 
        //thus leaving an umarked child. This will cause error when inv_flag is supposed to be set to 0x2
        child = CLR_ADPINV(curr->m_child[i]);
        //child = CLR_INVALID(child);
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
uint32_t MDList::DeleteMin()
{
    ASSERT_CODE(
            __sync_fetch_and_add(&g_count_del_attempt, 1);
            uint32_t repath = 0;
            uint32_t nullnode = 0;
            )
    uint32_t renode = 0;

    Node* min_node = NULL;
    Node* prg = NULL;
    Stack* old_s = m_stack;
    Stack* s = (Stack*)m_pool.AllocStack();
    *s = *CLR_MARKED(old_s);

    for(int dim = DIMENSION - 1; dim >= 0;)
    {
        ASSERT_CODE(repath++;)
        Node* last_min = s->m_del[dim];
        Desc* pending = last_min->m_pending;
        if(pending && pending->pred_dim <= dim && dim < pending->dim) FinishInserting(last_min, pending);

        Node* child = CLR_INVALID(last_min->m_child[dim]);
        if(child == NULL)
        {
            ASSERT_CODE(nullnode++;)
            --dim;
            continue;
        }

        prg = child->m_purged;
        if(IS_DELETED(prg))
        {
            //nodes before this one has been purged
            if(CLR_DELETED(prg) != NULL)
            {
                renode = 0;
                s->m_head = CLR_DELETED(prg);
                for(int i = DIMENSION - 1; i >= 0; --i)
                {
                    s->m_del[i] = s->m_head; 
                }
                dim = DIMENSION - 1;
            }
            else
            {
                renode++;

                for(int i = DIMENSION - 1; i >= dim; --i)
                {
                    s->m_del[i] = child; 
                }
                dim = DIMENSION - 1;
            }
        }
        else if(__sync_bool_compare_and_swap(&child->m_purged, prg, SET_DELETED(prg)))
        {
            for(int i = DIMENSION - 1; i >= dim; --i)
            {
                s->m_del[i] = child; 
            }

            __sync_bool_compare_and_swap(&m_stack, old_s, s);

            if((renode > 32) && (m_purge == NULL))
            {
                if(__sync_bool_compare_and_swap(&m_purge, NULL, s))
                {
                    if(s->m_head == m_head) Purge(s->m_head, s->m_del[DIMENSION - 1]);
                    m_purge = NULL;
                }
            }

            min_node = child;
            break;
        }
    }

    ASSERT_CODE(
            uint32_t old = g_count_maxrepath;
            if(repath > old) __sync_val_compare_and_swap(&g_count_maxrepath, old, repath);
            __sync_fetch_and_add(&g_count_avgrepath, repath);
            __sync_fetch_and_add(&g_count_avgrenode, renode);
            __sync_fetch_and_add(&g_count_avgnullnode, nullnode);
            )
    if(min_node)
    {
        ASSERT_CODE(
                __sync_fetch_and_sub(&g_count, 1);
                __sync_fetch_and_add(&g_count_del, 1);
                //printf("[%u] Pop node %p key %u\n", g_count_del_attempt, min_node, min_node->m_key);
                )
            return min_node->m_key;
    }
    else
    {
        //ASSERT_CODE(printf("[%u] Failed to pop\n", g_count_del_attempt);)
        return 0;
    }
}


inline void MDList::Purge(Node* head, Node* purged)
{

    ASSERT_CODE(__sync_fetch_and_add(&g_count_purge, 1);)

    //printf("Purging old head %p, last del %p\n", head, purged);
    Node* del_copy = (Node*)m_pool.AllocNode();
    Node* new_head = (Node*)m_pool.AllocNode();
    memset(del_copy->m_child, 0, sizeof(Node::m_child));
    memcpy(del_copy->m_coord, purged->m_coord, sizeof(Node::m_coord));
    del_copy->m_key = purged->m_key;
    del_copy->m_purged = purged->m_purged;
    del_copy->m_pending = NULL;
    memset(new_head, 0, sizeof(Node));
    new_head->m_purged = SET_DELETED(NULL);
    new_head->m_seq = head->m_seq + 1;
    ASSERT(new_head->m_seq > 0, "head seq overflow");

    uint32_t reset = 0;
    //printf("Purging head old/new %p/%p, purged/copy %p/%p\n", head, new_head, purged, del_copy);

    int purged_dim = -1;
    Node* curr = head;
    for(uint32_t dim =0; dim < DIMENSION;)
    {
        while(purged->m_coord[dim] > curr->m_coord[dim])
        {
            Desc* pending = curr->m_pending;
            if(pending && pending->pred_dim <= dim && dim < pending->dim) FinishInserting(curr, pending);
            Node* child = curr->m_child[dim];
            //if(IS_PRGINV(child)) ASSERT(!IS_ADPINV(CLR_INVALID(child)->m_child[dim]), "conflict adpinv for prginv node");
            curr = CLR_INVALID(child);
            ASSERT(curr, "curr must be valid during purge");
        }
        ASSERT(purged->m_coord[dim] == curr->m_coord[dim], "purged node should be reacheable from head");

        Desc* pending = curr->m_pending;
        if(pending && pending->pred_dim <= dim && dim < pending->dim) FinishInserting(curr, pending);
        Node* child = curr->m_child[dim];
        while(!IS_ADPINV(child) && !IS_PRGINV(child) && !__sync_bool_compare_and_swap(&curr->m_child[dim], child, SET_PRGINV(child)))
        {
            child = curr->m_child[dim];
        }

        //if(IS_ADPINV(child) && !IS_PRGINV(child)) 
        if(IS_ADPINV(child)) 
        {
            curr = head;
            dim = 0;
            continue; 
        }
        
        child = CLR_PRGINV(child);

        if(curr == head)
        {
            ASSERT_CODE(if(new_head->m_child[dim]) ASSERT(child == new_head->m_child[dim], "purge node should always be identical");)
            new_head->m_child[dim] = child;
            del_copy->m_child[dim] = SET_ADPINV(NULL);
            purged_dim = dim;
        }
        else
        {
            //ASSERT_CODE(if(del_copy->m_coord[dim] == 3) ASSERT(child == NULL, "no child for last node in a dim");)
            ASSERT_CODE(if(del_copy->m_child[dim]) ASSERT(child == del_copy->m_child[dim], "purge node should always be identical");)
            del_copy->m_child[dim] = child;
        }

        ++dim;
    }
    
    new_head->m_child[purged_dim + 1] = del_copy;
    head->m_purged = SET_DELETED(purged);
    purged->m_purged = SET_DELETED(new_head);
    m_head = new_head;

    //__sync_bool_compare_and_swap(&head->m_purged, SET_DELETED(NULL), SET_DELETED(purged));
    //__sync_bool_compare_and_swap(&purged->m_purged, SET_DELETED(NULL), SET_DELETED(new_head));
    //__sync_bool_compare_and_swap(&m_head, head, new_head);

    //printf("Done purging, new head %p, copy child %p\n", new_head, del_copy);
}



//------------------------------------------------------------------------------
inline void MDList::Traverse(Node* n, Node* parent, int dim, std::string& prefix)
{
    if(!IS_DELETED(n->m_purged))
    {
        printf("%s", prefix.c_str());
        printf("Node [%p] Key [%u] DIM [%d] of Parent[%p]\n", n, n->m_key, dim, parent);
    }

    //traverse from last dimension up to current dim
    //The valid children include child nodes up to dim
    //e.g. a node on dimension 3 has only valid children on dimensions 3~8
    for (int i = DIMENSION - 1; i >= dim; --i) 
    {
        //ASSERT(!IS_ADPINV(n->m_child[i]), "corrupt mdlist, invalid child in a valid dimension");
        if(IS_INVALID(n->m_child[i])) 
        {
            //ASSERT(false, "prginv child during traverse");
            continue;
        }

        if(n->m_child[i] != NULL)
        {
            prefix.push_back('|');
            prefix.insert(prefix.size(), i, ' ');

            Traverse(n->m_child[i], n, i, prefix);

            prefix.erase(prefix.size() - i - 1, i + 1);
        }
    }
}
