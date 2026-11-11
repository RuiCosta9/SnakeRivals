/**
 * @file hud.c
 * @brief Gameplay HUD drawing implementation.
 */

#include "hud.h"

#include <stdio.h>

#include "high_score.h"
#include "renderer.h"
#include "text.h"

int hud_draw_gameplay(const score_t *score, const snake_t *snake, uint16_t x, uint16_t y) {
  static char score_text[24];
  static char length_text[16];
  static char best_text[24];
  uint8_t scale = 3;

  snprintf(score_text, sizeof(score_text), "SCORE %u", (unsigned) score_get_value(score));

  if (renderer_submit_text(score_text, x, y, scale, 0x0F, RENDER_LAYER_UI) != 0)
    return 1;

  snprintf(length_text, sizeof(length_text), "LEN %u", (unsigned) snake_get_length(snake));

  uint16_t length_x = x + text_width(score_text, scale) + 32;
  if (renderer_submit_text(length_text, length_x, y, scale, 0x0E, RENDER_LAYER_UI) != 0)
    return 1;

  snprintf(best_text, sizeof(best_text), "BEST %u", (unsigned) high_score_get_value());
  uint16_t best_x = length_x + text_width(length_text, scale) + 32;
  if (renderer_submit_text(best_text, best_x, y, scale, 0x0B, RENDER_LAYER_UI) != 0)
    return 1;

  return 0;
}
