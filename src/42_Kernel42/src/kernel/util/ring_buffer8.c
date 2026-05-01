#include "kernel/util/ring_buffer8.h"


void rb_init(ring_buffer8_t* rb) {
  rb->head = 0;
  rb->tail = 0;
}

bool rb_push(ring_buffer8_t* rb, uint8_t data) {
  uint32_t current_tail = rb->tail;
  uint32_t next_head = (rb->head + 1) % ring_buffer_SIZE; // Wrap around to front if needed
  bool isFull = next_head == current_tail;

  if (isFull) {
    return false;
  }

  rb->buffer[rb->head] = data;
  rb->head = next_head;

  return true;
}


bool rb_pop(ring_buffer8_t* rb, uint8_t* out_data) {
  if (rb_is_empty(rb)) {
    return false;
  }

  *out_data = rb->buffer[rb->tail];

  rb->tail = (rb->tail + 1) % ring_buffer_SIZE;
  return true;
}
