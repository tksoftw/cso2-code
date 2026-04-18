#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "pool.h"

#define POOL_MAX_TASKS 200

struct Task {
    task_fn fn;
    void *arg;
    void *result;
    int done;
};

struct Pool {
    pthread_mutex_t lock;
    pthread_cond_t work_available;
    pthread_cond_t all_done;

    pthread_t *threads;
    int num_threads;

    struct Task tasks[POOL_MAX_TASKS];
    int submitted_count;
    int next_to_run;
    int completed_count;
    int stop_flag;
};

static struct Pool pool;
static bool pool_initialized = false;

static void *pool_worker(void *arg) {
    (void) arg;
    for (;;) {
        pthread_mutex_lock(&pool.lock);
        while (!pool.stop_flag && pool.next_to_run >= pool.submitted_count) {
            pthread_cond_wait(&pool.work_available, &pool.lock);
        }
        if (pool.next_to_run >= pool.submitted_count) {
            pthread_mutex_unlock(&pool.lock);
            return NULL;
        }
        int id = pool.next_to_run;
        pool.next_to_run += 1;
        task_fn fn = pool.tasks[id].fn;
        void *task_arg = pool.tasks[id].arg;
        pthread_mutex_unlock(&pool.lock);

        void *result = fn(task_arg);

        pthread_mutex_lock(&pool.lock);
        pool.tasks[id].result = result;
        pool.tasks[id].done = 1;
        pool.completed_count += 1;
        if (pool.completed_count == pool.submitted_count) {
            pthread_cond_broadcast(&pool.all_done);
        }
        pthread_mutex_unlock(&pool.lock);
    }
}

void pool_setup(int threads) {
    if (!pool_initialized) {
        pthread_mutex_init(&pool.lock, NULL);
        pthread_cond_init(&pool.work_available, NULL);
        pthread_cond_init(&pool.all_done, NULL);
        pool_initialized = true;
    }

    pool.submitted_count = 0;
    pool.next_to_run = 0;
    pool.completed_count = 0;
    pool.stop_flag = 0;
    for (int i = 0; i < POOL_MAX_TASKS; i += 1) {
        pool.tasks[i].fn = NULL;
        pool.tasks[i].arg = NULL;
        pool.tasks[i].result = NULL;
        pool.tasks[i].done = 0;
    }
    pool.num_threads = threads;
    pool.threads = malloc(sizeof(pthread_t) * (size_t) threads);

    for (int i = 0; i < threads; i += 1) {
        pthread_create(&pool.threads[i], NULL, pool_worker, NULL);
    }
}

int pool_submit_task(task_fn task, void *argument) {
    pthread_mutex_lock(&pool.lock);
    int id = pool.submitted_count;
    pool.submitted_count += 1;
    pool.tasks[id].fn = task;
    pool.tasks[id].arg = argument;
    pool.tasks[id].result = NULL;
    pool.tasks[id].done = 0;
    pthread_cond_signal(&pool.work_available);
    pthread_mutex_unlock(&pool.lock);
    return id;
}

void pool_wait(void) {
    pthread_mutex_lock(&pool.lock);
    int target = pool.submitted_count;
    while (pool.completed_count < target) {
        pthread_cond_wait(&pool.all_done, &pool.lock);
    }
    pthread_mutex_unlock(&pool.lock);
}

void *pool_get_task_result(int task_id) {
    pthread_mutex_lock(&pool.lock);
    void *result = NULL;
    if (task_id >= 0 && task_id < pool.submitted_count && pool.tasks[task_id].done) {
        result = pool.tasks[task_id].result;
    }
    pthread_mutex_unlock(&pool.lock);
    return result;
}

void pool_stop(void) {
    pthread_mutex_lock(&pool.lock);
    pool.stop_flag = 1;
    pthread_cond_broadcast(&pool.work_available);
    pthread_mutex_unlock(&pool.lock);

    for (int i = 0; i < pool.num_threads; i += 1) {
        pthread_join(pool.threads[i], NULL);
    }

    free(pool.threads);
    pool.threads = NULL;
    pool.num_threads = 0;
    //DONT destroy mutex
}