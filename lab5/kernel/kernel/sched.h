#ifndef SCHED_H_
#define SCHED_H_

#include "kthread.h"

void schedule();
void sched_enqueue_task(kthread_t *thread);
void sched_init();

#endif // SCHED_H_
