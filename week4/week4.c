#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ITEM_COUNT 5

typedef struct {
    int values[ITEM_COUNT];
} Buffer;

typedef struct {
    Buffer *input;
    Buffer *output;
    int delay_ms;
} StageData;

static void sleep_ms(int milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (long)(milliseconds % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static uint32_t next_random(uint32_t *state)
{
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

static int next_delay(uint32_t *state)
{
    return 50 + (int)(next_random(state) % 251u);
}

static void print_buffer(const char *label, const Buffer *buffer)
{
    printf("%s", label);
    for (int i = 0; i < ITEM_COUNT; i++) {
        printf(" %d", buffer->values[i]);
    }
    printf("\n");
}

static void *stage1(void *argument)
{
    StageData *data = (StageData *)argument;
    sleep_ms(data->delay_ms);
    //(void)data;

    /* TODO 1: write values 1..5 to the output buffer and print it. */
    for(int i = 0; i < ITEM_COUNT; i++){
        data->output->values[i] = i + 1;
    }

    print_buffer("Stage 1 data:", data->output);

    return NULL;
}

static void *stage2(void *argument)
{
    StageData *data = (StageData *)argument;
    sleep_ms(data->delay_ms);
    (void)data;

    /* TODO 2: read the input buffer, write squares to the output buffer and print it. */
    for(int i = 0; i < ITEM_COUNT; i++){
        int value = data->input->values[i];
        data->output->values[i] = value*value;
    }

    print_buffer("Stage 2 data:", data->output);

    return NULL;
}

static void *stage3(void *argument)
{
    StageData *data = (StageData *)argument;
    sleep_ms(data->delay_ms);
    (void)data;

    /* TODO 3: calculate and print the sum of the input buffer. */
    int sum = 0;
    for(int i = 0; i < ITEM_COUNT; i++){
        sum += data->input->values[i];
    }

    printf("Result sum = %d\n", sum);

    return NULL;
}

static int run_part_a(StageData *s1, StageData *s2, StageData *s3)
{
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    (void)thread1;
    (void)thread2;
    (void)thread3;
    (void)s1;
    (void)s2;
    (void)s3;

    /* TODO 4: create and join one stage at a time: 1 -> 2 -> 3. */
    pthread_create(&thread1, NULL, stage1, s1);
    pthread_join(thread1, NULL);

    pthread_create(&thread2, NULL, stage2, s2);
    pthread_join(thread2, NULL);

    pthread_create(&thread3, NULL, stage3, s3);
    pthread_join(thread3, NULL);

    return EXIT_SUCCESS;
}

static int run_part_b(StageData *s1, StageData *s2, StageData *s3)
{
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    (void)thread1;
    (void)thread2;
    (void)thread3;
    (void)s1;
    (void)s2;
    (void)s3;

    /* TODO 5: create all three stages before the first join. */
    pthread_create(&thread1, NULL, stage1, s1);
    pthread_create(&thread2, NULL, stage2, s2);
    pthread_create(&thread3, NULL, stage3, s3);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    (void)print_buffer;
    (void)stage1;
    (void)stage2;
    (void)stage3;

    if (argc < 2 || argc > 3 ||
        (argv[1][0] != 'A' && argv[1][0] != 'B') ||
        argv[1][1] != '\0') {
        fprintf(stderr, "Usage: %s A|B [seed]\n", argv[0]);
        return EXIT_FAILURE;
    }

    uint32_t seed = (argc == 3)
                        ? (uint32_t)strtoul(argv[2], NULL, 10)
                        : (uint32_t)time(NULL);

    int delay1 = next_delay(&seed);
    int delay2 = next_delay(&seed);
    int delay3 = next_delay(&seed);

    Buffer buffer1 = {0};
    Buffer buffer2 = {0};

    StageData stage1_data = {NULL, &buffer1, delay1};
    StageData stage2_data = {&buffer1, &buffer2, delay2};
    StageData stage3_data = {&buffer2, NULL, delay3};

    if (argv[1][0] == 'A') {
        return run_part_a(&stage1_data, &stage2_data, &stage3_data);
    }

    return run_part_b(&stage1_data, &stage2_data, &stage3_data);
}