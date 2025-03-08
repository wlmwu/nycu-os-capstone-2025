#include "irq.h"
#include "reg_interrupt.h"
#include "mini_uart.h"
#include "timer.h"
#include "priority_queue.h"
#include "utils.h"

static priority_queue_t *irq_task_queue;
static irq_task_t *irq_handling_task;

static bool irq_task_comp(void *newn, void *n) {
    return ((irq_task_t*)newn)->priority < ((irq_task_t*)n)->priority;
}

void irq_init() {
    irq_task_queue = (priority_queue_t*)malloc(sizeof(priority_queue_t));
    pq_init(irq_task_queue, irq_task_comp);
    irq_handling_task = NULL;
    irq_enable();
}

void irq_enable(){
    __asm__ __volatile__("msr daifclr, 0x2");
}
void irq_disable(){
    __asm__ __volatile__("msr daifset, 0x2");
}

void irq_handle() {
    irq_disable();

    irq_task_t *task = NULL;

    if ((*IRQ_PENDING_1 & AUX_INT) &&                                                 // AUX interrupt
    (*((volatile unsigned int*)(CORE0_INTERRUPT_SOURCE)) & INTERRUPT_SOURCE_GPU)) {   // GPU interrupt
        // UART interrupt
        // https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf  P. 112
        // https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf  P. 16

        uart_irq_disable();  
        task = irq_task_construct(uart_irq_handle, NULL, 0, 0);

    } else if ((*((volatile unsigned int*)(CORE0_INTERRUPT_SOURCE)) & INTERRUPT_SOURCE_CNTPNSIRQ)) {    // CNTPNSIRQ interrupt
        // CNTPNSIRQ (Non-Secure Physical Timer Interrupt): Timer interrupt
        // https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf  P. 16
        
        timer_irq_disable();
        task = irq_task_construct(timer_irq_handle, NULL, 0, 1);

    } else {
        uart_printf("IRQ_PENDING_1: %x\n", *IRQ_PENDING_1);
        uart_printf("CORE0_INTERRUPT_SOURCE: %x\n\n", *((volatile unsigned int*)(CORE0_INTERRUPT_SOURCE)));
    }
    
    if (task) {
        pq_push(irq_task_queue, task);
    }
    
    irq_enable();
    irq_handle_pending_tasks();
}

irq_task_t* irq_task_construct(irq_task_fn_t fn, void *args, size_t argsize, uint64_t priority) {
    irq_task_t *task = (irq_task_t*)malloc(sizeof(irq_task_t));
    irq_task_fn_t handle_fn = fn;
    task->handle_fn = handle_fn;
    task->priority = priority;
    if (args && argsize > 0) {
        task->args = malloc(argsize);
        memcpy(task->args, args, argsize);
    } else {
        task->args = NULL;
    }
    return task;
}

void irq_task_destruct(irq_task_t *task) {
    free(task->args);
    free(task);
}

void irq_handle_pending_tasks() {
    while (!pq_empty(irq_task_queue)) {
        irq_task_t *task = (irq_task_t*)pq_top(irq_task_queue);
        if (irq_handling_task && !irq_task_comp(task, irq_handling_task)) {
            break;
        }
        irq_disable();  // Critical section start

        pq_pop(irq_task_queue);
        irq_handling_task = task;
        
        irq_enable();   // Critical section end

        irq_handling_task->handle_fn(irq_handling_task->args);
        irq_task_destruct(irq_handling_task);
        irq_handling_task = NULL;
    }
}