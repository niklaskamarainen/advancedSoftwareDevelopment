#ifndef WORKLOAD_H
#define WORKLOAD_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Valmis infrastruktuuri: opiskelijan ei tarvitse muuttaa tätä tiedostoa. */

void busy_work_ms(int milliseconds);
void timespec_add_ms(struct timespec *t, int milliseconds);
double timespec_diff_ms(const struct timespec *start, const struct timespec *end);
int deadline_missed(const struct timespec *release_time,
                    const struct timespec *finish_time,
                    int deadline_ms);

const char *policy_name(int policy);
void print_task_config(const char *name, int policy, int priority);
void print_job_result(const char *name, int job_number, int missed);
void print_summary(const char *name, int jobs, int misses);

#ifdef __cplusplus
}
#endif

#endif
