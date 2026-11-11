#ifndef HUD_H
#define HUD_H

#include <stdint.h>

#include "score.h"
#include "snake.h"

/**
 * @file hud.h
 * @brief Immediate HUD drawing helpers for gameplay screens.
 */

/**
 * @brief Draw gameplay score/length information.
 *
 * @param score Score to display.
 * @param snake Snake used for length information.
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @return 0 on success, non-zero on failure.
 */
int hud_draw_gameplay(const score_t *score, const snake_t *snake, uint16_t x, uint16_t y);

#endif
