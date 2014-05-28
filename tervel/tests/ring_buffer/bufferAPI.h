
#ifdef USE_WF_BUFFER
    #include "test_buffers/wfbuffer_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif


#ifdef USE_ATOMIC_ONLY
    #include "test_buffers/FAA_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_COARSE_LOCK
    #include "test_buffers/lock_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_MCAS_BUFFER
    #include "test_buffers/mcas_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_TBB_QUEUE
    #include "test_buffers/tbb_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_TSIGAS_QUEUE
    #include "test_buffers/tsigas_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif


#ifdef USE_LINUX_BUFFER
    #include "test_buffers/linux_API.h"
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
