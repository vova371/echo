#ifndef	ECHO_COMMON_H
#define ECHO_COMMON_H

#include <time.h>

static __inline__ uint64_t checkpoint()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static __inline__ void error_check(int ret, const char* text)
{
    if (ret == -1)
    {
        printf("error %s: %s\n", text, strerror(errno));
        exit(1);
    }
}

#endif
