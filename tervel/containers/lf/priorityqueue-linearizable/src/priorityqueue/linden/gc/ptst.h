/******************************************************************************
 * ptst.h
 * 
 * Per-thread state management.
 *
 *
 * Copyright (c) 2013, Jonatan Linden
 * Copyright (c) 2002-2003, K A Fraser
 */

#ifndef __PTST_H__
#define __PTST_H__

typedef struct ptst_st ptst_t;

#include <gsl/gsl_rng.h>

#include "gc.h"

struct ptst_st
{
    /* Thread id */
    unsigned int id;
    /* State management */
    ptst_t      *next;
    unsigned int count;

    /* Utility structures */
    gc_t        *gc;
    char pad[56];
    unsigned int rand;
};

 /*
 * Enter/leave a critical region. A thread gets a state handle for
 * use during critical regions.
 */

void critical_enter(void );

#define critical_exit() gc_exit(ptst)

/* Iterators */
extern ptst_t *ptst_list;

#define ptst_first()  (ptst_list)
#define ptst_next(_p) ((_p)->next)



#endif /* __PTST_H__ */



