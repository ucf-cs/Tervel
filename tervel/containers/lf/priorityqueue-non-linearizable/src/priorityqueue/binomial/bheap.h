/* iheap.h -- Binomial Heaps with integer priorities
 *
 * Copyright (c) 2008, Bjoern B. Brandenburg <bbb [at] cs.unc.edu>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of North Carolina nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY  COPYRIGHT OWNER AND CONTRIBUTERS ``AS IS'' AND
 * ANY  EXPRESS OR  IMPLIED  WARRANTIES,  INCLUDING, BUT  NOT  LIMITED TO,  THE
 * IMPLIED WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO  EVENT SHALL THE  COPYRIGHT OWNER OR  CONTRIBUTERS BE
 * LIABLE  FOR  ANY  DIRECT,   INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
 * CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED  TO,  PROCUREMENT  OF
 * SUBSTITUTE GOODS  OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
 * INTERRUPTION)  HOWEVER CAUSED  AND ON  ANY THEORY  OF LIABILITY,  WHETHER IN
 * CONTRACT,  STRICT LIABILITY,  OR  TORT (INCLUDING  NEGLIGENCE OR  OTHERWISE)
 * ARISING IN ANY WAY  OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef IHEAP_H
#define IHEAP_H

#define NOT_IN_HEAP UINT_MAX

struct iheap_node {
    struct iheap_node*  parent;
    struct iheap_node*  next;
    struct iheap_node*  child;

    unsigned int        degree;
    int         key;
    const void*         value;
    struct iheap_node** ref;
};

struct iheap {
    struct iheap_node*  head;
    /* We cache the minimum of the heap.
     * This speeds up repeated peek operations.
     */
    struct iheap_node*  min;
};


static inline void iheap_init(struct iheap* heap)
{
    heap->head = NULL;
    heap->min  = NULL;
}

static inline void iheap_node_init_ref(struct iheap_node** _h,
                      int key, const void* value)
{
    struct iheap_node* h = *_h;
    h->parent = NULL;
    h->next   = NULL;
    h->child  = NULL;
    h->degree = NOT_IN_HEAP;
    h->value  = value;
    h->ref    = _h;
    h->key    = key;
}

static inline void iheap_node_init(struct iheap_node* h, int key, const void* value)
{
    h->parent = NULL;
    h->next   = NULL;
    h->child  = NULL;
    h->degree = NOT_IN_HEAP;
    h->value  = value;
    h->ref    = NULL;
    h->key    = key;
}

static inline const void* iheap_node_value(struct iheap_node* h)
{
    return h->value;
}

static inline int iheap_node_in_heap(struct iheap_node* h)
{
    return h->degree != NOT_IN_HEAP;
}

static inline int iheap_empty(struct iheap* heap)
{
    return heap->head == NULL && heap->min == NULL;
}

/* make child a subtree of root */
static inline void __iheap_link(struct iheap_node* root,
                   struct iheap_node* child)
{
    child->parent = root;
    child->next   = root->child;
    root->child   = child;
    root->degree++;
}

/* merge root lists */
static inline struct iheap_node* __iheap_merge(struct iheap_node* a,
                         struct iheap_node* b)
{
    struct iheap_node* head = NULL;
    struct iheap_node** pos = &head;

    while (a && b) {
        if (a->degree < b->degree) {
            *pos = a;
            a = a->next;
        } else {
            *pos = b;
            b = b->next;
        }
        pos = &(*pos)->next;
    }
    if (a)
        *pos = a;
    else
        *pos = b;
    return head;
}

/* reverse a linked list of nodes. also clears parent pointer */
static inline struct iheap_node* __iheap_reverse(struct iheap_node* h)
{
    struct iheap_node* tail = NULL;
    struct iheap_node* next;

    if (!h)
        return h;

    h->parent = NULL;
    while (h->next) {
        next    = h->next;
        h->next = tail;
        tail    = h;
        h       = next;
        h->parent = NULL;
    }
    h->next = tail;
    return h;
}

static inline void __iheap_min(struct iheap* heap,
                  struct iheap_node** prev,
                  struct iheap_node** node)
{
    struct iheap_node *_prev, *cur;
    *prev = NULL;

    if (!heap->head) {
        *node = NULL;
        return;
    }

    *node = heap->head;
    _prev = heap->head;
    cur   = heap->head->next;
    while (cur) {
        if (cur->key < (*node)->key) {
            *node = cur;
            *prev = _prev;
        }
        _prev = cur;
        cur   = cur->next;
    }
}

static inline void __iheap_union(struct iheap* heap, struct iheap_node* h2)
{
    struct iheap_node* h1;
    struct iheap_node *prev, *x, *next;
    if (!h2)
        return;
    h1 = heap->head;
    if (!h1) {
        heap->head = h2;
        return;
    }
    h1 = __iheap_merge(h1, h2);
    prev = NULL;
    x    = h1;
    next = x->next;
    while (next) {
        if (x->degree != next->degree ||
            (next->next && next->next->degree == x->degree)) {
            /* nothing to do, advance */
            prev = x;
            x    = next;
        } else if (x->key < next->key) {
            /* x becomes the root of next */
            x->next = next->next;
            __iheap_link(x, next);
        } else {
            /* next becomes the root of x */
            if (prev)
                prev->next = next;
            else
                h1 = next;
            __iheap_link(next, x);
            x = next;
        }
        next = x->next;
    }
    heap->head = h1;
}

static inline struct iheap_node* __iheap_extract_min(struct iheap* heap)
{
    struct iheap_node *prev, *node;
    __iheap_min(heap, &prev, &node);
    if (!node)
        return NULL;
    if (prev)
        prev->next = node->next;
    else
        heap->head = node->next;
    __iheap_union(heap, __iheap_reverse(node->child));
    return node;
}

/* insert (and reinitialize) a node into the heap */
static inline void iheap_insert(struct iheap* heap, struct iheap_node* node)
{
    struct iheap_node *min;
    node->child  = NULL;
    node->parent = NULL;
    node->next   = NULL;
    node->degree = 0;
    if (heap->min && node->key < heap->min->key) {
        /* swap min cache */
        min = heap->min;
        min->child  = NULL;
        min->parent = NULL;
        min->next   = NULL;
        min->degree = 0;
        __iheap_union(heap, min);
        heap->min   = node;
    } else
        __iheap_union(heap, node);
}

static inline void __iheap_uncache_min(struct iheap* heap)
{
    struct iheap_node* min;
    if (heap->min) {
        min = heap->min;
        heap->min = NULL;
        iheap_insert(heap, min);
    }
}

/* merge addition into target */
static inline void iheap_union(struct iheap* target, struct iheap* addition)
{
    /* first insert any cached minima, if necessary */
    __iheap_uncache_min(target);
    __iheap_uncache_min(addition);
    __iheap_union(target, addition->head);
    /* this is a destructive merge */
    addition->head = NULL;
}

static inline struct iheap_node* iheap_peek(struct iheap* heap)
{
    if (!heap->min)
        heap->min = __iheap_extract_min(heap);
    return heap->min;
}

static inline struct iheap_node* iheap_take(struct iheap* heap)
{
    struct iheap_node *node;
    if (!heap->min)
        heap->min = __iheap_extract_min(heap);
    node = heap->min;
    heap->min = NULL;
    if (node)
        node->degree = NOT_IN_HEAP;
    return node;
}

static inline void iheap_decrease(struct iheap* heap, struct iheap_node* node,
                  int new_key)
{
    struct iheap_node *parent;
    struct iheap_node** tmp_ref;
    const void* tmp;
    int   tmp_key;

    /* node's priority was decreased, we need to update its position */
    if (!node->ref || new_key >= node->key)
        return;
    node->key = new_key;
    if (heap->min != node) {
        if (heap->min && node->key < heap->min->key)
            __iheap_uncache_min(heap);
        /* bubble up */
        parent = node->parent;
        while (parent && node->key < parent->key) {
            /* swap parent and node */
            tmp           = parent->value;
            tmp_key       = parent->key;
            parent->value = node->value;
            parent->key   = node->key;
            node->value   = tmp;
            node->key     = tmp_key;
            /* swap references */
            if (parent->ref)
                *(parent->ref) = node;
            *(node->ref)   = parent;
            tmp_ref        = parent->ref;
            parent->ref    = node->ref;
            node->ref      = tmp_ref;
            /* step up */
            node   = parent;
            parent = node->parent;
        }
    }
}

static inline void iheap_delete(struct iheap* heap, struct iheap_node* node)
{
    struct iheap_node *parent, *prev, *pos;
    struct iheap_node** tmp_ref;
    const void* tmp;
    int tmp_key;

    if (!node->ref) /* can only delete if we have a reference */
        return;
    if (heap->min != node) {
        /* bubble up */
        parent = node->parent;
        while (parent) {
            /* swap parent and node */
            tmp           = parent->value;
            tmp_key       = parent->key;
            parent->value = node->value;
            parent->key   = node->key;
            node->value   = tmp;
            node->key     = tmp_key;
            /* swap references */
            if (parent->ref)
                *(parent->ref) = node;
            *(node->ref)   = parent;
            tmp_ref        = parent->ref;
            parent->ref    = node->ref;
            node->ref      = tmp_ref;
            /* step up */
            node   = parent;
            parent = node->parent;
        }
        /* now delete:
         * first find prev */
        prev = NULL;
        pos  = heap->head;
        while (pos != node) {
            prev = pos;
            pos  = pos->next;
        }
        /* we have prev, now remove node */
        if (prev)
            prev->next = node->next;
        else
            heap->head = node->next;
        __iheap_union(heap, __iheap_reverse(node->child));
    } else
        heap->min = NULL;
    node->degree = NOT_IN_HEAP;
}

#endif /* HEAP_H */
