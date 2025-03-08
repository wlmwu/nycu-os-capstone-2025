#include "ringbuffer.h"
#include "utils.h"

ring_buffer_t* ring_buffer_init(char *buf, size_t buf_size) {
    if (!(RING_BUFFER_IS_POWER_OF_TWO(buf_size))) {   // Size is not power of two
        return NULL;
    }
    ring_buffer_t *buffer = malloc(sizeof(ring_buffer_t));
    buffer->buffer = buf;
    buffer->buffer_mask = buf_size - 1;
    buffer->tail_index = 0;
    buffer->head_index = 0;

    return buffer;
}

void ring_buffer_enqueue(ring_buffer_t *buffer, char data) {
    /* Is buffer full? */
    if(ring_buffer_is_full(buffer)) {
      /* Is going to overwrite the oldest byte */
      /* Increase tail index */
      buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK(buffer));
    }
  
    /* Place data in buffer */
    buffer->buffer[buffer->head_index] = data;
    buffer->head_index = ((buffer->head_index + 1) & RING_BUFFER_MASK(buffer));
}

uint8_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data) {
    if(ring_buffer_is_empty(buffer)) {
      /* No items */
      return 0;
    }
  
    *data = buffer->buffer[buffer->tail_index];
    buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK(buffer));
    return 1;
}

uint8_t ring_buffer_get(ring_buffer_t *buffer, char *data, size_t index) {
    if(index >= ring_buffer_size(buffer)) {
      /* No items at index */
      return 0;
    }
  
    /* Add index to pointer */
    size_t data_index = ((buffer->tail_index + index) & RING_BUFFER_MASK(buffer));
    *data = buffer->buffer[data_index];
    return 1;
}

extern inline uint8_t ring_buffer_is_full(ring_buffer_t *buffer);
extern inline uint8_t ring_buffer_is_empty(ring_buffer_t *buffer);
extern inline size_t ring_buffer_size(ring_buffer_t *buffer);