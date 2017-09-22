#define _GNU_SOURCE
#include "common.h"

#if defined(__linux__)
pid_t 
gettid(void) 
{
    return (pid_t) syscall(SYS_gettid);
}

void
pin(pid_t t, int cpu) 
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    E_en(sched_setaffinity(t, sizeof(cpu_set_t), &cpuset));
}

void
gettime(struct timespec *ts)
{
    E(clock_gettime(CLOCK_MONOTONIC, ts));
}

#endif

#if defined(__APPLE__)
void
gettime(struct timespec *ts)
{
    uint64_t time = mach_absolute_time();

    static mach_timebase_info_data_t info = {0,0};

    if (info.denom == 0)  {
	mach_timebase_info(&info);
    }

    uint64_t elapsed = time * (info.numer / info.denom);

    ts->tv_sec = elapsed * 1e-9;
    ts->tv_nsec = elapsed - (ts->tv_sec * 1e9);
}
#endif




struct timespec
timediff (struct timespec begin, struct timespec end)
{
    struct timespec tmp;
    if ((end.tv_nsec - begin.tv_nsec) < 0) {
	tmp.tv_sec = end.tv_sec - begin.tv_sec - 1;
	tmp.tv_nsec = 1000000000 + end.tv_nsec - begin.tv_nsec;
    } else {
	tmp.tv_sec = end.tv_sec - begin.tv_sec;
	tmp.tv_nsec = end.tv_nsec - begin.tv_nsec;
    }
    return tmp;
}
