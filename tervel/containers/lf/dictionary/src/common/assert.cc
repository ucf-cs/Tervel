//------------------------------------------------------------------------------
// 
//     SST/Macro tools set
//
//------------------------------------------------------------------------------

#include <cstdio>
#include <cstdarg>
#include "common/assert.h"

#if defined ENABLE_ASSERT

namespace detail{

enum
{
    ASSERT_MESSAGE_SIZE = 512
};

void Assert::ConditionFailed(const char * condition, const char * message, ... )
{
    char formatedMessage[ ASSERT_MESSAGE_SIZE ];

    va_list arguments;
    va_start( arguments, message );
    vsnprintf( &formatedMessage[0], ASSERT_MESSAGE_SIZE, message, arguments );    
    va_end( arguments );

    fprintf(stderr, "\n**********************\nCondition Failed: %s\nMessage: %s\n**********************\n", condition, formatedMessage );

    //Crash here in order to have a decent
    //call stack in the crash handler's output
    //Simply calling ::DebugBreak destroy the 
    //existing call stack
    __asm__ ( "int $0x03" );
}

}

#endif // ENABLE_ASSERT
