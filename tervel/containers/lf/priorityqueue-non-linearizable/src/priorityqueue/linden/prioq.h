#ifndef PRIOQ_H
#define PRIOQ_H

#include "common.h"

typedef unsigned long pkey_t;
typedef unsigned int  pval_t;

#define KEY_NULL 0
#define NUM_LEVELS 32
/* Internal key values with special meanings. */
#define SENTINEL_KEYMIN ( 0UL) /* Key value of first dummy node. */
#define SENTINEL_KEYMAX (~1UL) /* Key value of last dummy node.  */


typedef struct node_s
{
    pkey_t    k;
    int       level;
    int       inserting; //char pad2[4];
    pval_t    v;
    struct node_s *next[1];
} node_t;

typedef struct
{
    int    max_offset;
    int    max_level;
    int    nthreads;
    node_t *head;
    node_t *tail;
    char   pad[128];
} pq_t;

#define get_marked_ref(_p)      ((void *)(((uintptr_t)(_p)) | 1))
#define get_unmarked_ref(_p)    ((void *)(((uintptr_t)(_p)) & ~1))
#define is_marked_ref(_p)       (((uintptr_t)(_p)) & 1)


/* Interface */

extern pq_t *pq_init(int max_offset);

extern void pq_destroy(pq_t *pq);

extern void insert(pq_t *pq, pkey_t k, pval_t v);

extern pval_t deletemin(pq_t *pq);

extern void sequential_length(pq_t *pq);

#endif // PRIOQ_H
