#ifndef SCORE_H
#define SCORE_H

#include <stdint.h>

/**
 * @file score.h
 * @brief Score value and multiplier helpers.
 */

#define SCORE_FOOD_POINTS 10
#define SCORE_DEFAULT_MULTIPLIER 1

/**
 * @brief Mutable score with a temporary multiplier.
 */
typedef struct {
  uint32_t value;    /**< Current score value. */
  uint8_t multiplier;/**< Multiplier applied by score_add_food(). */
} score_t;

/**
 * @brief Initialize a score to zero with the default multiplier.
 *
 * @param score Score to initialize.
 */
void score_init(score_t *score);

/**
 * @brief Reset a score to zero with the default multiplier.
 *
 * @param score Score to reset.
 */
void score_reset(score_t *score);

/**
 * @brief Add the default food points using the active multiplier.
 *
 * @param score Score to update.
 */
void score_add_food(score_t *score);

/**
 * @brief Add an arbitrary number of points without applying the multiplier.
 *
 * @param score Score to update.
 * @param points Points to add.
 */
void score_add_points(score_t *score, uint16_t points);

/**
 * @brief Set the score multiplier.
 *
 * @param score Score to update.
 * @param multiplier New multiplier.
 */
void score_set_multiplier(score_t *score, uint8_t multiplier);

/**
 * @brief Get the current score value.
 *
 * @param score Score to inspect.
 * @return Score value, or 0 for NULL.
 */
uint32_t score_get_value(const score_t *score);

/**
 * @brief Get the current score multiplier.
 *
 * @param score Score to inspect.
 * @return Multiplier, or SCORE_DEFAULT_MULTIPLIER for NULL.
 */
uint8_t score_get_multiplier(const score_t *score);

#endif
