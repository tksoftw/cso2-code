#ifndef POOL_H_
#define POOL_H_

// The function signature for a task.
//
// A task should be a function like:
//
//     void *my_task(void*) { ... }
//
typedef void*(*task_fn)(void*);

// Initialize the thread pool.
void pool_setup(int threads);

// Submit a new task to the thread pool and return its ID.
int pool_submit_task(task_fn task, void *argument);

// Get the return value from a task.
void *pool_get_task_result(int task_id);

// Return after all existing tasks have finished.
void pool_wait(void);

// Tell the thread pool threads to finish any current task and then exit.
void pool_stop(void);

#endif