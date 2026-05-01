#pragma once
#include <stdbool.h>

#include "stdint.h"


#define ring_buffer_SIZE 256

/**
 * A circular buffer for unsigned 8 bit values.
 */
typedef struct {
  uint8_t buffer[ring_buffer_SIZE];
  volatile uint32_t head;
  volatile uint32_t tail;
} ring_buffer8_t;

/**
 * Zeroes out the ringbuffer
 * @param rb ringbuffer
 */
void rb_init(ring_buffer8_t* rb);

/**
 * Push a byte to the ringbuffer
 * @param rb ringbuffer
 * @param data byte to push
 * @return false if buffer is full
 */
bool rb_push(ring_buffer8_t* rb, uint8_t data);

/**
 * pop a value from the ringbuffer
 * @param rb ringbuffer
 * @param out_data data popped
 * @return false if buffer is empty
 */
bool rb_pop(ring_buffer8_t* rb, uint8_t* out_data);

/**
 * Checks if the ringbuffer is empty
 * @param rb ringbuffer
 * @return true if its empty
 */
inline __attribute__((always_inline)) bool rb_is_empty(const ring_buffer8_t* rb) {
  return rb->head == rb->tail;
}
