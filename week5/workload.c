#define _POSIX_C_SOURCE 200809L
#include "workload.h"

#include <sched.h>
#include <stdio.h>
#include <time.h>

static long long to_ns(const struct timespec *t)
{
    return (long long)t->tv_sec * 1000000000LL + (long long)t->tv_nsec;
}

void busy_work_ms(int milliseconds)
{
    struct timespec start;
    struct timespec now;
    const long long target_ns = (long long)milliseconds * 1000000LL;

    clock_gettime(CLOCK_MONOTONIC, &start);

    do {
        /* CPU-bound workload: tarkoituksella ei sleep-kutsua. */
        clock_gettime(CLOCK_MONOTONIC, &now);
    } while (to_ns(&now) - to_ns(&start) < target_ns);
}

void timespec_add_ms(struct timespec *t, int milliseconds)
{
    t->tv_sec += milliseconds / 1000;
    t->tv_nsec += (long)(milliseconds % 1000) * 1000000L;

    if (t->tv_nsec >= 1000000000L) {
        t->tv_sec += 1;
        t->tv_nsec -= 1000000000L;
    }
}

double timespec_diff_ms(const struct timespec *start, const struct timespec *end)
{
    long long diff_ns = to_ns(end) - to_ns(start);
    return (double)diff_ns / 1000000.0;
}

int deadline_missed(const struct timespec *release_time,
                    const struct timespec *finish_time,
                    int deadline_ms)
{
    return timespec_diff_ms(release_time, finish_time) > (double)deadline_ms;
}

const char *policy_name(int policy)
{
    switch (policy) {
    case SCHED_FIFO:
        return "SCHED_FIFO";
    case SCHED_RR:
        return "SCHED_RR";
    case SCHED_OTHER:
        return "SCHED_OTHER";
    default:
        return "UNKNOWN";
    }
}

void print_task_config(const char *name, int policy, int priority)
{
    printf("W5 POLICY %s %s\n", name, policy_name(policy));
    printf("W5 PRIORITY %s %d\n", name, priority);
}

void print_job_result(const char *name, int job_number, int missed)
{
    printf("W5 JOB %s %d %s\n",
           name,
           job_number,
           missed ? "MISS" : "OK");
}

void print_summary(const char *name, int jobs, int misses)
{
    printf("W5 SUMMARY %s jobs=%d misses=%d\n", name, jobs, misses);
}
