/**
 * @file serial_queue.c
 * @brief Circular byte queue implementation for serial RX buffering.
 */

#include "serial_queue.h"

#include <stddef.h>

void serial_queue_init(serial_queue_t *queue) {
  serial_queue_clear(queue);
}

void serial_queue_clear(serial_queue_t *queue) {
  if (queue == NULL)
    return;

  queue->head = 0;
  queue->tail = 0;
  queue->size = 0;
}

bool serial_queue_is_empty(const serial_queue_t *queue) {
  return queue == NULL || queue->size == 0;
}

bool serial_queue_is_full(const serial_queue_t *queue) {
  return queue != NULL && queue->size >= SERIAL_QUEUE_CAPACITY;
}

uint16_t serial_queue_size(const serial_queue_t *queue) {
  if (queue == NULL)
    return 0;

  return queue->size;
}

int serial_queue_push(serial_queue_t *queue, uint8_t byte) {
  if (queue == NULL)
    return 1;

  if (serial_queue_is_full(queue))
    return 1;

  queue->data[queue->head] = byte;
  queue->head = (uint16_t) ((queue->head + 1) % SERIAL_QUEUE_CAPACITY);
  queue->size++;

  return 0;
}

int serial_queue_pop(serial_queue_t *queue, uint8_t *byte) {
  if (queue == NULL || byte == NULL)
    return 1;

  if (serial_queue_is_empty(queue))
    return 1;

  *byte = queue->data[queue->tail];
  queue->tail = (uint16_t) ((queue->tail + 1) % SERIAL_QUEUE_CAPACITY);
  queue->size--;

  return 0;
}
