#ifndef TIMER_H
#define TIMER_H

#include <lcom/lcf.h>

#include <stdint.h>

/**
 * @file timer.h
 * @brief i8254 timer driver reused by the project.
 */

/**
 * @brief Set a timer frequency.
 *
 * @param timer Timer index.
 * @param freq Target frequency.
 * @return 0 on success, non-zero on failure.
 */
int(timer_set_frequency)(uint8_t timer, uint32_t freq);

/**
 * @brief Read a timer configuration byte.
 *
 * @param timer Timer index.
 * @param st Output status byte.
 * @return 0 on success, non-zero on failure.
 */
int(timer_get_conf)(uint8_t timer, uint8_t *st);

/**
 * @brief Display timer configuration using LCF.
 *
 * @param timer Timer index.
 * @param st Status byte.
 * @param field Field to display.
 * @return 0 on success, non-zero on failure.
 */
int(timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field);

/**
 * @brief Subscribe timer interrupts.
 *
 * @param bit_no Output interrupt bit number.
 * @return 0 on success, non-zero on failure.
 */
int(timer_subscribe_int)(uint8_t *bit_no);

/**
 * @brief Unsubscribe timer interrupts.
 *
 * @return 0 on success, non-zero on failure.
 */
int(timer_unsubscribe_int)(void);

/**
 * @brief Timer interrupt handler.
 */
void(timer_int_handler)(void);

/**
 * @brief Reset the internal timer tick counter.
 */
void timer_reset_counter(void);

/**
 * @brief Get the internal timer tick counter.
 *
 * @return Number of timer interrupts handled.
 */
unsigned int timer_get_counter(void);

#endif
