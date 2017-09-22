#ifndef __PORTABLE_DEFNS_H__
#define __PORTABLE_DEFNS_H__

#define MAX_THREADS 128 /* Nobody will ever have more! */

#if defined(SPARC)
#include "sparc_defns.h"
#elif defined(INTEL)
#include "intel_defns.h"
#elif defined(PPC)
#include "ppc_defns.h"
#elif defined(IA64)
#include "ia64_defns.h"
#elif defined(MIPS)
#include "mips_defns.h"
#elif defined(ALPHA)
#include "alpha_defns.h"
#else
#error "A valid architecture has not been defined"
#endif

#include <string.h>

#ifndef MB_NEAR_CAS
#define RMB_NEAR_CAS() RMB()
#define WMB_NEAR_CAS() WMB()
#define MB_NEAR_CAS()  MB()
#endif

typedef unsigned long int_addr_t;

typedef int bool_t;
#define FALSE 0
#define TRUE  1

#define ADD_TO(_v,_x)                                                   \
do {                                                                    \
    int __val = (_v), __newval;                                         \
    while ( (__newval = CASIO(&(_v),__val,__val+(_x))) != __val )       \
        __val = __newval;                                               \
} while ( 0 )

/*
 * Allow us to efficiently align and pad structures so that shared fields
 * don't cause contention on thread-local or read-only fields.
 */
#define CACHE_PAD(_n) char __pad ## _n [CACHE_LINE_SIZE]
#define ALIGNED_ALLOC(_s)                                       \
    ((void *)(((unsigned long)malloc((_s)+CACHE_LINE_SIZE*2) +  \
        CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE-1)))


/*
 * POINTER MARKING
 */
#define get_marked_ref(_p)      ((void *)(((unsigned long)(_p)) | 1))
#define get_unmarked_ref(_p)    ((void *)(((unsigned long)(_p)) & ~1))
#define is_marked_ref(_p)       (((unsigned long)(_p)) & 1)



/* Read field @_f into variable @_x. */
#define READ_FIELD(_x,_f) ((_x) = (_f))

#define WEAK_DEP_ORDER_RMB() ((void)0)
#define WEAK_DEP_ORDER_WMB() ((void)0)
#define WEAK_DEP_ORDER_MB()  ((void)0)



#endif /* __PORTABLE_DEFNS_H__ */
