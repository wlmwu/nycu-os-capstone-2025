#ifndef KTHREAD_H_
#define KTHREAD_H_

#include <stddef.h>
#include <stdint.h>
#include "sched.h"

sched_task_t *kthread_create(sched_fn_t fn, void *args);
void kthread_exit();

sched_task_t *kthread_run(sched_fn_t fn, void *args);

#endif // KTHREAD_H_
