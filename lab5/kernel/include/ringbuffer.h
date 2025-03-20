// Modified from: https://github.com/AndersKaloer/Ring-Buffer/

#include <stdint.h>
#include <stddef.h>

/**
 * Checks if the buffer_size is a power of two.
 * Due to the design only <tt> RING_BUFFER_SIZE-1 </tt> items
 * can be contained in the buffer.
 * buffer_size must be a power of two.
*/
#define RING_BUFFER_IS_POWER_OF_TWO(buffer_size) ((buffer_size & (buffer_size - 1)) == 0)

/**
 * Used as a modulo operator
 * as <tt> a % b = (a & (b − 1)) </tt>
 * where \c a is a positive index in the buffer and
 * \c b is the (power of two) size of the buffer.
 */
#define RING_BUFFER_MASK(rb) (rb->buffer_mask)

typedef struct ring_buffer {
    /** Buffer memory. */
    char *buffer;
    /** Buffer mask. (Size - 1) */
    size_t buffer_mask;
    /** Index of tail. */
    size_t tail_index;
    /** Index of head. */
    size_t head_index;

} ring_buffer_t;
  
/**
 * Initializes the ring buffer. The resulting buffer can contain <em>buf_size-1</em> bytes.
 * @param buf The buffer allocated for the ringbuffer.
 * @param buf_size The size of the allocated ringbuffer. It should be the power of 2.
 * @return A pointer to a static `ring_buffer_t` buffer.
 *         Returns NULL if memory allocation fails.
 */
ring_buffer_t* ring_buffer_init(char *buf, size_t buf_size);

/**
 * Adds a byte to a ring buffer.
 * @param buffer The buffer in which the data should be placed.
 * @param data The byte to place.
 */
void ring_buffer_enqueue(ring_buffer_t *buffer, char data);

/**
 * Returns the oldest byte in a ring buffer.
 * @param buffer The buffer from which the data should be returned.
 * @param data A pointer to the location at which the data should be placed.
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data);

/**
 * Peeks a ring buffer, i.e. returns an element without removing it.
 * @param buffer The buffer from which the data should be returned.
 * @param data A pointer to the location at which the data should be placed.
 * @param index The index to peek.
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t ring_buffer_get(ring_buffer_t *buffer, char *data, size_t index);

/**
 * Returns whether a ring buffer is full.
 * @param buffer The buffer for which it should be returned whether it is full.
 * @return 1 if full; 0 otherwise.
 */
inline uint8_t ring_buffer_is_full(ring_buffer_t *buffer) {
    return ((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK(buffer)) == RING_BUFFER_MASK(buffer);
}

/**
 * Returns whether a ring buffer is empty.
 * @param buffer The buffer for which it should be returned whether it is empty.
 * @return 1 if empty; 0 otherwise.
 */
inline uint8_t ring_buffer_is_empty(ring_buffer_t *buffer) {
    return (buffer->head_index == buffer->tail_index);
}

/**
 * Returns the number of items in a ring buffer.
 * @param buffer The buffer for which the number of items should be returned.
 * @return The number of items in the ring buffer.
 */
inline size_t ring_buffer_size(ring_buffer_t *buffer) {
    return ((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK(buffer));
}
  