#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "workload.h"

typedef struct {
    const char *name;
    int period_ms;
    int deadline_ms;
    int workload_ms;
    int jobs;
    int priority;
} task_config_t;

static task_config_t sensor = {
    .name = "SENSOR",
    .period_ms = 100,
    .deadline_ms = 100,
    .workload_ms = 20,
    .jobs = 20,
    .priority = 0
};

static task_config_t control = {
    .name = "CONTROL",
    .period_ms = 250,
    .deadline_ms = 200,
    .workload_ms = 35,
    .jobs = 8,
    .priority = 0
};

static task_config_t diagnostics = {
    .name = "DIAGNOSTICS",
    .period_ms = 500,
    .deadline_ms = 500,
    .workload_ms = 50,
    .jobs = 4,
    .priority = 0
};

static void fail_pthread(const char *what, int err)
{
    fprintf(stderr, "%s: %s\n", what, strerror(err));
    exit(EXIT_FAILURE);
}

static void *task_worker(void *arg)
{
    task_config_t *task = (task_config_t *)arg;
    int actual_policy = 0;
    struct sched_param actual_param;
    int misses = 0;

    /* TODO A: tarkista tämän säikeen todellinen scheduling policy ja priority
       opetuksessa käytetyllä pthread-mekanismilla. */

    int rc = pthread_getschedparam(pthread_self(), &actual_policy, &actual_param);
    if(rc != 0)
    {
        fail_pthread("pthread_getschedparam", rc);
    }

    print_task_config(task->name, actual_policy, actual_param.sched_priority);

    struct timespec next_release;
    (void)next_release;

    /* TODO B: ota CLOCK_MONOTONIC-ajasta ensimmäinen release-hetki. */

    clock_gettime(CLOCK_MONOTONIC, &next_release);

    for (int job = 1; job <= task->jobs; ++job) {
        struct timespec release_time;
        struct timespec finish_time;
        (void)release_time;
        (void)finish_time;

        /* TODO C: toteuta absoluuttiseen aikaan perustuva periodinen suoritus.
           - odota seuraavaan release-hetkeen
           - tallenna release_time
           - suorita valmiiksi annettu workload
           - tallenna finish_time
           - käytä valmista deadline-tarkistusta
           - päivitä next_release seuraavaa jobia varten */

        rc = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                             &next_release, NULL);
        if (rc != 0)
        {
            fprintf(stderr, "clock_nanosleep: %s\n", strerror(rc));
            return NULL;
        }

        release_time = next_release;

        busy_work_ms(task->workload_ms);

        clock_gettime(CLOCK_MONOTONIC, &finish_time);
        

        int missed = deadline_missed(&release_time, &finish_time, task->deadline_ms); /* korvaa oikealla deadline-tarkistuksen tuloksella */
        if (missed) {
            ++misses;
        }

        timespec_add_ms(&next_release, task->period_ms);

        print_job_result(task->name, job, missed);
    }

    print_summary(task->name, task->jobs, misses);
    return NULL;
}

static void configure_attr(pthread_attr_t *attr, int priority)
{
    struct sched_param param;
    memset(&param, 0, sizeof(param));

    int rc = pthread_attr_init(attr);
    if (rc != 0) {
        fail_pthread("pthread_attr_init", rc);
    }

    /* TODO D: määritä tälle pthread-attribuutille
       - SCHED_FIFO
       - annettu priority
       - eksplisiittinen scheduling
       käyttäen yhteisessä koodausharjoituksessa käytettyjä mekanismeja. */

    rc = pthread_attr_setschedpolicy(attr, SCHED_FIFO);
    if (rc != 0)
        fail_pthread("pthread_attr_setschedpolicy", rc);

    param.sched_priority = priority;
    rc = pthread_attr_setschedparam(attr, &param);
    if (rc != 0)
        fail_pthread("pthread_attr_setschedparam", rc);

    rc = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
    if (rc != 0)
        fail_pthread("pthread_attr_setinheritsched", rc);


    
    (void)priority;
    (void)param;
}

int main(void)
{
    pthread_t sensor_thread;
    pthread_t control_thread;
    pthread_t diagnostics_thread;

    pthread_attr_t sensor_attr;
    pthread_attr_t control_attr;
    pthread_attr_t diagnostics_attr;

    (void)sensor_thread;
    (void)control_thread;
    (void)diagnostics_thread;
    (void)task_worker;

    /* TODO E: selvitä Linuxilta SCHED_FIFO-politiikan sallittu
       prioriteettialue. Älä kovakoodaa rajoja. */
    int min_priority = sched_get_priority_min(SCHED_FIFO);
    int max_priority = sched_get_priority_max(SCHED_FIFO);

    if (min_priority == -1 || max_priority == -1) {
        perror("sched_get_priority_min/max");
        return EXIT_FAILURE;
    }

    printf("W5 PRIORITY_RANGE min=%d max=%d\n", min_priority, max_priority);

    /* TODO F: muodosta RMS-prioriteetit niin, että
       SENSOR > CONTROL > DIAGNOSTICS ja kaikki arvot ovat sallitulla alueella. */

    if(max_priority - 2 < min_priority)
    {
        fprintf(stderr, "priority levels out of range\n");
        return EXIT_FAILURE;
    }

    sensor.priority = max_priority;
    control.priority = max_priority - 1;
    diagnostics.priority = max_priority - 2;

    

    configure_attr(&sensor_attr, sensor.priority);
    configure_attr(&control_attr, control.priority);
    configure_attr(&diagnostics_attr, diagnostics.priority);

    /* TODO G: luo kolme säiettä oikeilla attribuuteilla ja task-konfiguraatioilla.
       Käytä task_worker-funktiota kaikkien kolmen säikeen workerina. */

    int rc = pthread_create(&sensor_thread, &sensor_attr, task_worker, &sensor);
    if (rc == EPERM)
    {
        fprintf(stderr,
                "pthread_create: EPERM. RT scheduling ei ole sallittu.\n");
        pthread_attr_destroy(&sensor_attr);
        return EXIT_FAILURE;
    }
    if (rc != 0)
    {
        pthread_attr_destroy(&sensor_attr);
        fail_pthread("pthread_create", rc);
    }


    rc = pthread_create(&control_thread, &control_attr, task_worker, &control);
    if (rc == EPERM)
    {
        fprintf(stderr,
                "pthread_create: EPERM. RT scheduling ei ole sallittu.\n");
        pthread_attr_destroy(&control_attr);
        return EXIT_FAILURE;
    }
    if (rc != 0)
    {
        pthread_attr_destroy(&control_attr);
        fail_pthread("pthread_create", rc);
    }


    rc = pthread_create(&diagnostics_thread, &diagnostics_attr, task_worker, &diagnostics);
    if (rc == EPERM)
    {
        fprintf(stderr,
                "pthread_create: EPERM. RT scheduling ei ole sallittu.\n");
        pthread_attr_destroy(&diagnostics_attr);
        return EXIT_FAILURE;
    }
    if (rc != 0)
    {
        pthread_attr_destroy(&diagnostics_attr);
        fail_pthread("pthread_create", rc);
    }



    /* TODO H: odota kaikkien kolmen säikeen päättymistä. */

    rc = pthread_join(sensor_thread, NULL);
    if (rc != 0) {
        fail_pthread("pthread_join", rc);
    }

    rc = pthread_join(control_thread, NULL);
    if (rc != 0) {
        fail_pthread("pthread_join", rc);
    }

    rc = pthread_join(diagnostics_thread, NULL);
    if (rc != 0) {
        fail_pthread("pthread_join", rc);
    }


    pthread_attr_destroy(&sensor_attr);
    pthread_attr_destroy(&control_attr);
    pthread_attr_destroy(&diagnostics_attr);

    return 0;
}
