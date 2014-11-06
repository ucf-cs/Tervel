#ifdef USE_WF_HASHMAP
    #include "api/wf_hashmap_api.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_WF_OLD_HASHMAP
    #include "api/wf_hashmap_old_api.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_TBB_HASHMAP
#include "api/tbb_map.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_BOOST_HASHMAP
#include "api/lock_boost_map.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_STL_HASHMAP
#include "api/lock_stl_map.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_SPLIT_HASHMAP
#include "api/cds_split_map.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_CLIFF_HASHMAP
#include "api/cliff_api.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_MICHAEL_HASHMAP
#include "api/cds_michael_map.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifndef V_API_SANITY
    #error no buffer APIs enabled
#else
    #undef V_API_SANITY
#endif
