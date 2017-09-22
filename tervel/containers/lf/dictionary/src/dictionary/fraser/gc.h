#ifndef __GC_H__
#define __GC_H__

typedef struct gc_st gc_t;

/* Most of these functions peek into a per-thread state struct. */
#include "ptst.h"

/* Initialise GC section of given per-thread state structure. */
gc_t *fr_gc_init(void);

int fr_gc_add_allocator(int alloc_size);
void fr_gc_remove_allocator(int alloc_id);

/*
 * Memory allocate/free. An unsafe free can be used when an object was
 * not made visible to other processes.
 */
void *fr_gc_alloc(ptst_t *ptst, int alloc_id);
void fr_gc_free(ptst_t *ptst, void *p, int alloc_id);
void fr_gc_unsafe_free(ptst_t *ptst, void *p, int alloc_id);

/*
 * Hook registry. Allows users to hook in their own per-epoch delay
 * lists.
 */
typedef void (*hook_fn_t)(ptst_t *, void *);
int fr_gc_add_hook(hook_fn_t fn);
void fr_gc_remove_hook(int hook_id);
void fr_gc_add_ptr_to_hook_list(ptst_t *ptst, void *ptr, int hook_id);

/* Per-thread entry/exit from critical regions */
void fr_gc_enter(ptst_t *ptst);
void fr_gc_exit(ptst_t *ptst);

/* Start-of-day initialisation of garbage collector. */
void fr_init_gc_subsystem(void);
void fr_destroy_gc_subsystem(void);

#endif /* __GC_H__ */
