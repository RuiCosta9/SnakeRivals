/**
 * @file high_score.c
 * @brief Runtime high-score storage implementation.
 */

#include "high_score.h"

#include <stdbool.h>
#include <stdint.h>

static uint32_t best_score = 0;

bool high_score_submit(uint32_t score) {
  if (score <= best_score)
    return false;

  best_score = score;
  return true;
}

uint32_t high_score_get_value(void) {
  return best_score;
}

void high_score_reset(void) {
  best_score = 0;
}
