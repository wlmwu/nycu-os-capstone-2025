#include "list.h"
#include <stdbool.h>
#include <stdint.h>

// Priority queue node (embeds list_head)
struct pq_node {
    void *data;             // Pointer to the stored data
    struct list_head list;  // Linked list node
};

typedef bool (*pq_comp_fn_t)(void *new_insert, void *n);

typedef struct priority_queue {
    struct list_head head;  // Head of the list (dummy node)
    pq_comp_fn_t comp;      // Compare method
} priority_queue_t;

static inline void pq_init(priority_queue_t *pq, pq_comp_fn_t comp) {
    INIT_LIST_HEAD(&pq->head);
    pq->comp = comp;
}

static inline bool pq_empty(priority_queue_t *pq) {
    return list_empty(&pq->head);
}


void pq_push(priority_queue_t *pq, void *data);
void *pq_pop(priority_queue_t *pq);
void *pq_top(priority_queue_t *pq);
