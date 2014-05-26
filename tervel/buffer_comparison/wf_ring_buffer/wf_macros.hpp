#ifndef __WF_MACROS__
#define __WF_MACROS__ 1

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

#define HELP_DELAY 10000
#define MAX_FAILURES 100000
#define NOT_VALUE 0x0

#define ALIGNLEN 128


typedef std::atomic<void *> ArrayElement;

extern __thread long fcount;
extern __thread int rDepth;
extern __thread int threadID;

//WF/Loop limitation
//__thread OpRecord *myOp = NULL; //must be set before first call to Helper::remove, 
//extern __thread std::atomic<void *> *controlWord;//when not null, return back to own operation.

static std::atomic<int> activeThreads;




#endif
