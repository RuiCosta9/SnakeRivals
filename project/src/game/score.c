/**
 * @file score.c
 * @brief Score value and multiplier implementation.
 */

#include "score.h"

#include <stddef.h>

void score_init(score_t *score) {
  score_reset(score);
}

void score_reset(score_t *score) {
  if (score == NULL)
    return;

  score->value = 0;
  score->multiplier = SCORE_DEFAULT_MULTIPLIER;
}

void score_add_food(score_t *score) {
  score_add_points(score, SCORE_FOOD_POINTS);
}

void score_add_points(score_t *score, uint16_t points) {
  if (score == NULL)
    return;

  score->value += (uint32_t) points * score->multiplier;
}

void score_set_multiplier(score_t *score, uint8_t multiplier) {
  if (score == NULL)
    return;

  score->multiplier = multiplier == 0 ? SCORE_DEFAULT_MULTIPLIER : multiplier;
}

uint32_t score_get_value(const score_t *score) {
  if (score == NULL)
    return 0;

  return score->value;
}

uint8_t score_get_multiplier(const score_t *score) {
  if (score == NULL)
    return SCORE_DEFAULT_MULTIPLIER;

  return score->multiplier;
}
