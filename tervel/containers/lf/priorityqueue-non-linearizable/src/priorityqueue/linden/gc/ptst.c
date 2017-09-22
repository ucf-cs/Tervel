/******************************************************************************
 * ptst.c
 * 
 * Per-thread state management. Essentially the state management parts
 * of MB's garbage-collection code have been pulled out and placed
 * here, for the use of other utility routines.
 * 
 * Copyright (c) 2013, Jonatan Linden
 * Copyright (c) 2002-2003, K A Fraser
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 
 *  * The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "random.h"
#include "portable_defns.h"
#include "ptst.h"

ptst_t *ptst_list = NULL;
extern __thread ptst_t *ptst;
static unsigned int next_id = 0;

void
critical_enter()
{
    ptst_t *next, *new_next;

    if ( ptst == NULL ) 
    {
	ptst = (ptst_t *) ALIGNED_ALLOC(sizeof(ptst_t));
	if ( ptst == NULL ) exit(1);
	    
	memset(ptst, 0, sizeof(ptst_t));
	ptst->gc = gc_init();
	ptst->count = 1;
	ptst->id = __sync_fetch_and_add(&next_id, 1);
	rand_init(ptst);
	new_next = ptst_list;
	do {
	    ptst->next = next = new_next;
	} 
	while ( (new_next = __sync_val_compare_and_swap(&ptst_list, next, ptst)) != next );
    }
    
    gc_enter(ptst);
    return;
}



static void ptst_destructor(ptst_t *ptst) 
{
    ptst->count = 0;
}


