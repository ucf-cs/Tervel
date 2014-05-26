#include <atomic>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <memory>

#ifdef DEF_PB_LOCK
	#include <mutex>
#endif

#define MAX_ARRAYS 64
#define HELP_DELAY 10000
#define MAX_FAILURES 100000
#define NOT_VALUE 0x0
#define DEAD_NOT_VALUE 0x1
#define NOT_ALIVE_VALUE (NOT_VALUE|(0x3))

#define ALIGNLEN 128


typedef std::atomic<void *> ArrayElement;
//typedef ArrayElement* ArrayType;

//extern __thread long fcount;
__thread long fcount=0;
__thread int rDepth=0;
__thread int threadID;

//WF/Loop limitation
//__thread OpRecord *myOp = NULL; //must be set before first call to Helper::remove, 
//__thread std::atomic<void *> *controlWord(NULL);//when not null, return back to own operation.

//static std::atomic<int> activeThreads;
