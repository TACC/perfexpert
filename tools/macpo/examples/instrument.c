#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS         8
#define ITERATIONS          128
#define RANDOM_BUFFER_SIZE  51200000
#define REPEAT_COUNT        81290

typedef struct {
    int tid;
} thread_info_t;

int counts[NUM_THREADS];
float random_numbers[RANDOM_BUFFER_SIZE];

void* thread_func(void* arg) {
    int idx, i, repeat;
    float x, y, z;
    thread_info_t* thread_info = (thread_info_t*) arg;

    for (repeat = 0; repeat < REPEAT_COUNT; repeat++) {
        for (i = 0; i < ITERATIONS; i++) {
            idx = i + thread_info->tid;
            x = random_numbers[idx % RANDOM_BUFFER_SIZE];
            y = random_numbers[(1 + idx) % RANDOM_BUFFER_SIZE];
            z = x * x + y * y;
            if (z <= 1) counts[thread_info->tid]++;
        }
    }

    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    int t;
    pthread_t threads[NUM_THREADS];
    thread_info_t thread_info[NUM_THREADS];

    fprintf(stdout, "creating threads...\n");
    for (t = 0; t < NUM_THREADS; t++) {
        thread_info[t].tid = t;
        int rc = pthread_create(&threads[t], NULL, thread_func,
                (void *) &thread_info[t]);

        if (rc) {
            fprintf(stdout, "ERROR; return code from pthread_create() is %d\n",
                    rc);
            return -1;
        }
    }

    for (t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    fprintf(stdout, "all threads terminated, returning from main()\n");
    return 0;
}
