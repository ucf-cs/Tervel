//------------------------------------------------------------------------------
// 
//     
//
//------------------------------------------------------------------------------

#ifndef _COMMON_UTILITIES_ASSERT_INCLUDED_
#define _COMMON_UTILITIES_ASSERT_INCLUDED_

#include "config.h"

//------------------------------------------------------------------------
// Assert is used to emit run time error
//------------------------------------------------------------------------
#if defined ENABLE_ASSERT
    namespace detail {
        class Assert
        {
        public:
            static void ConditionFailed( const char * condition, const char * message, ... );
        };
    }

    #define ASSERT( condition, ... ) ( !(condition) ? detail::Assert::ConditionFailed( #condition, __VA_ARGS__ ) : ((void)0) )
    #define ASSERT_CODE( assertCode ) assertCode
#else
    #define ASSERT( condition, ... ) ((void)sizeof(condition))
    #define ASSERT_CODE( assertCode )
#endif

//------------------------------------------------------------------------
//Static Assert used to emit compile time error
//------------------------------------------------------------------------
#ifdef ENABLE_STATIC_ASSERT
    // Based on Loki library!
    namespace detail
    {
        template< int >
        struct CompileTimeError;

        template<>
        struct CompileTimeError< true >{};
    }

    #define STATIC_ASSERT( expression, message ) \
        { detail::CompileTimeError<( (expression) != 0 ) > ERROR_##message; (void)ERROR_##message; } 
#else
    #define STATIC_ASSERT( expression, message ) ((void)0)
#endif

#endif //_COMMON_UTILITIES_ASSERT_INCLUDED_
