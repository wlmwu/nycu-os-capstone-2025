#include "priority_queue.h"
#include "utils.h"
#include <stdint.h>

void pq_push(priority_queue_t *pq, void *data) {
    struct pq_node *new_node = malloc(sizeof(struct pq_node));
    new_node->data = data;
    INIT_LIST_HEAD(&new_node->list);

    // If the queue is empty, add the node directly
    if (pq_empty(pq)) {
        list_add_tail(&new_node->list, &pq->head);
        return;
    }

    // Traverse the list to find the insertion point
    struct pq_node *entry;
    list_for_each_entry(entry, &pq->head, list) {
        // true: higher priority
        if (pq->comp(new_node->data, entry->data)) {
            list_add_tail(&new_node->list, &entry->list);
            return;
        }
    }

    // If all nodes have higher/equal priority, add to the end
    list_add_tail(&new_node->list, &pq->head);
}

void *pq_pop(priority_queue_t *pq) {
    if (pq_empty(pq)) {
        return NULL;
    }

    struct pq_node *node = list_entry(pq->head.next, struct pq_node, list);
    void *data = node->data;

    list_del(&node->list);
    free(node);

    return data;
}

void *pq_top(priority_queue_t *pq) {
    if (pq_empty(pq)) {
        return NULL;
    }

    struct pq_node *node = list_entry(pq->head.next, struct pq_node, list);
    return node->data;
}