#include "life.h"
#include <pthread.h>
#include <stdlib.h>

typedef struct {
    int thread_id;
    int start_y;
    int end_y;
    int steps;
    LifeBoard *state;
    LifeBoard *next_state;
    pthread_barrier_t *barrier;
} ThreadArgs;


void *life_worker(void *args_ptr) {
    ThreadArgs *args = (ThreadArgs *)args_ptr;

    for (int step = 0; step < args->steps; step += 1) {
        for (int y = args->start_y; y < args->end_y; y += 1) {
            for (int x = 1; x < args->state->width - 1; x += 1) {
                int live_in_window = 0;
                for (int y_offset = -1; y_offset <= 1; y_offset += 1)
                    for (int x_offset = -1; x_offset <= 1; x_offset += 1)
                        if (LB_get(args->state, x + x_offset, y + y_offset))
                            live_in_window += 1;

                LB_set(args->next_state, x, y,
                    live_in_window == 3 ||
                    (live_in_window == 4 && LB_get(args->state, x, y))
                );
            }
        }

        pthread_barrier_wait(args->barrier);

        if (args->thread_id == 0)
            LB_swap(args->state, args->next_state);

        pthread_barrier_wait(args->barrier);
    }

    return NULL;
}


void simulate_life_parallel(int threads, LifeBoard *state, int steps) {
    LifeBoard *next_state = LB_new(state->width, state->height);

    pthread_t tids[threads];
    ThreadArgs args[threads];

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, threads);

    int rows = state->height - 2; // inner rows: [1, height-1)
    int chunk = (rows + threads - 1) / threads;

    for (int i = 0; i < threads; i++) {
        args[i].thread_id = i;
        args[i].start_y = 1 + i * chunk;
        args[i].end_y = args[i].start_y + chunk;
        if (args[i].end_y > state->height - 1)
            args[i].end_y = state->height - 1;
        args[i].steps = steps;
        args[i].state = state;
        args[i].next_state = next_state;
        args[i].barrier = &barrier;

        pthread_create(&tids[i], NULL, life_worker, &args[i]);
    }

    for (int i = 0; i < threads; i++)
        pthread_join(tids[i], NULL);

    pthread_barrier_destroy(&barrier);
    LB_del(next_state);
}
