/*
#The MIT License (MIT)
#
#Copyright (c) 2015 University of Central Florida's Computer Software Engineering
#Scalable & Secure Systems (CSE - S3) Lab
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.
#
*/

#ifdef USE_WF_HASHMAP_ND
    #include "api/wf_hashmap_api.h"
    #ifdef V_API_SANITY
        #error TWO or more buffer APIs enabled
    #else
        #define V_API_SANITY
    #endif
#endif
#ifdef USE_WF_HASHMAP
    #include "api/wf_hashmapi_nodel_api.h"
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
