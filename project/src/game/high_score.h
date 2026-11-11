#ifndef HIGH_SCORE_H
#define HIGH_SCORE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file high_score.h
 * @brief In-memory high score tracker for the current run.
 */

/**
 * @brief Submit a score as a possible high score.
 *
 * @param score Score value to compare.
 * @return true if the stored high score changed.
 */
bool high_score_submit(uint32_t score);

/**
 * @brief Get the stored high score.
 *
 * @return Current high score value.
 */
uint32_t high_score_get_value(void);

/**
 * @brief Reset the stored high score to zero.
 */
void high_score_reset(void);

#endif
