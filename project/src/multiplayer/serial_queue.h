#ifndef SERIAL_QUEUE_H
#define SERIAL_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file serial_queue.h
 * @brief Fixed-size circular byte queue for serial RX buffering.
 */

#define SERIAL_QUEUE_CAPACITY 1024

/**
 * @brief Circular queue used to store received serial bytes.
 */
typedef struct {
  uint8_t data[SERIAL_QUEUE_CAPACITY]; /**< Queue storage. */
  uint16_t head;                       /**< Pop index. */
  uint16_t tail;                       /**< Push index. */
  uint16_t size;                       /**< Number of stored bytes. */
} serial_queue_t;

/**
 * @brief Initialize an empty queue.
 *
 * @param queue Queue to initialize.
 */
void serial_queue_init(serial_queue_t *queue);

/**
 * @brief Remove all bytes from a queue.
 *
 * @param queue Queue to clear.
 */
void serial_queue_clear(serial_queue_t *queue);

/**
 * @brief Check whether a queue is empty.
 *
 * @param queue Queue to inspect.
 * @return true if empty.
 */
bool serial_queue_is_empty(const serial_queue_t *queue);

/**
 * @brief Check whether a queue is full.
 *
 * @param queue Queue to inspect.
 * @return true if full.
 */
bool serial_queue_is_full(const serial_queue_t *queue);

/**
 * @brief Get the number of bytes currently stored.
 *
 * @param queue Queue to inspect.
 * @return Queue size.
 */
uint16_t serial_queue_size(const serial_queue_t *queue);

/**
 * @brief Push one byte into a queue.
 *
 * @param queue Queue to update.
 * @param byte Byte to push.
 * @return 0 on success, non-zero if full or invalid.
 */
int serial_queue_push(serial_queue_t *queue, uint8_t byte);

/**
 * @brief Pop one byte from a queue.
 *
 * @param queue Queue to update.
 * @param byte Output byte.
 * @return 0 on success, non-zero if empty or invalid.
 */
int serial_queue_pop(serial_queue_t *queue, uint8_t *byte);

#endif
