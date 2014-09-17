#ifdef USE_WF_VECTOR
    #include "api/wf_vector_api.h"
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
