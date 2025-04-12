#ifndef KTHREAD_H_
#define KTHREAD_H_

#include <stddef.h>
#include <stdint.h>
#include "sched.h"

/**
 * @brief Creates a new kernel thread but does not add it to the scheduler's run queue.
 * The created thread is not runnable until it is explicitly added to the scheduler's queue.
 *
 * @param fn    Entry point function for the new kernel thread.
 * @param args  Arguments to be passed to the entry point function.
 * @return      Pointer to the newly created `sched_task_t` on success, NULL on failure.
 */
sched_task_t *kthread_create(sched_fn_t fn, void *args);
void kthread_exit();

/**
 * @brief Creates a new kernel thread and adds it to the scheduler's run queue.
 *
 * @param fn    Entry point function for the new kernel thread.
 * @param args  Arguments to be passed to the entry point function.
 * @return      Pointer to the newly created sched_task_t on success, NULL on failure.
 */
sched_task_t *kthread_run(sched_fn_t fn, void *args);

#endif // KTHREAD_H_
