
#ifdef USE_WF_BUFFER
    #include "wf_ring_buffer/wf_ring_buffer_API.hpp"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_PADDED_WF_BUFFER
    #include "wf_ring_buffer_padded/wf_ring_buffer_API.hpp"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_ATOMIC_ONLY
    #include "only_atomic_FAA/only_atomic_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_COARSE_LOCK
    #include "coarse_lock_buffer/coarse_lock_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_MCAS_BUFFER
    #include "mcas_buffer/mcas_buffer_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_TBB_QUEUE
    #include "tbb_queue/tbb_queue_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_TSIGAS_QUEUE
    #include "tsigas_queue/tsigas_queue_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif


#ifdef USE_LINUX_BUFFER
    #include "linux_buffer/linux_buffer_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifdef USE_LINUX_BUFFER_RECREATE
    #include "linux_buffer_recreate/linux_buffer_recreate_API.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif

#ifndef V_API_SANITY
    #error no buffer APIs enabled
#else
    #undef SANITY
#endif
