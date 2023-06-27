#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

static __inline__ void cpuid(void)
{
  asm volatile("cpuid" ::: "%rax", "%rbx", "%rcx", "%rdx");
}

static __inline__ uint64_t rdtsc(void)
{
  uint32_t a, d; 
  asm volatile("rdtsc" : "=a" (a), "=d" (d));
  return ((uint64_t)a) | (((uint64_t)d) << 32); 
}

static __inline__ uint64_t timespec_to_nanos(struct timespec const* ts)
{
  return ts->tv_sec * 1000000000 + ts->tv_nsec;
}

#if 0

int main()
{
    int count=10;
    uint64_t val[count];

    for (int i=0; i<count; i++)
    {
        unsigned int h, l;
        __asm__ __volatile__("rdtscp" : "=a" (l), "=d" (h));
        val[i] = ((uint64_t)l) | (((uint64_t)h) << 32);
    }

    for (int i=0; i<count; i++)
        printf("%d: %lu\n", i, val[i]);

    return 0;
}

#else

int main(int argc, char* argv[])
{
    struct timespec ts;
    struct timespec tm;
    struct timespec tm0_s, tm0_e;
    struct timeval tv;

    int count = argc > 1 ? atoi(argv[1]) : 1000000;

    uint64_t cp0 = 0;
    uint64_t cp0_s = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &tm0_s);

    for (int i = 0; i<count; ++i)
    {
        // cpuid();
        // gettimeofday(&tv, NULL);
        // cp0 = rdtsc();
        // clock_gettime(CLOCK_REALTIME, &ts);
        clock_gettime(CLOCK_MONOTONIC, &tm);
    }

    uint64_t cp0_e = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &tm0_e);

    uint64_t nanos = timespec_to_nanos(&tm0_e) - timespec_to_nanos(&tm0_s);
    uint64_t cycles = cp0_e - cp0_s;
    double cpn = (double)cycles / (double)nanos;
    double cpi = (double)cycles / (double)count;
    double npi = (double)nanos / (double)count;

    printf("warmup [%d]: %ldns = %ld cycles, %g cycles per ns\n", count, nanos, cycles, cpn);
    printf("warmup loop: %g cycles per iteration\n", cpi);
    printf("warmup loop: %g nanos per iteration\n", npi);

    if (cp0)
        printf("warmup loop: %ld last clock diff\n", cp0_e - cp0);

#if 0

    uint64_t cp1_s = rdtsc();
    gettimeofday(&tv, NULL);
    uint64_t cp1_e = rdtsc();

    uint64_t cp2_s = rdtsc();
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t cp2_e = rdtsc();

    uint64_t cp3_s = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &tm);
    uint64_t cp3_e = rdtsc();

    printf("gettimeofday: %ld.%06ld - %ld\n", tv.tv_sec, tv.tv_usec, cp1_e - cp1_s);
    printf("CLOCK_REALTIME: %ld.%09ld - %ld\n", ts.tv_sec, ts.tv_nsec, cp2_e - cp2_s);
    printf("CLOCK_MONOTONIC: %ld.%09ld - %ld\n", tm.tv_sec, tm.tv_nsec, cp3_e - cp3_s);

#endif

    return 0;
}

#endif
