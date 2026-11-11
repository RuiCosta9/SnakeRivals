/**
 * @file renderer_overlays.c
 * @brief Pause and game-over overlay rendering.
 */

#include "renderer_internal.h"
#include "renderer.h"

#include <stdint.h>
#include <stdio.h>

#include "game_over_state.h"
#include "high_score.h"
#include "pause_state.h"
#include "score.h"
#include "singleplayer_state.h"
#include "text.h"
#include "ui_layout.h"
#include "vbe.h"

int renderer_draw_pause(void) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  if (renderer_draw_singleplayer() != 0)
    return 1;

  ui_rect_t first_button = ui_layout_get_pause_button(PAUSE_BUTTON_RESUME);
  ui_rect_t last_button = ui_layout_get_pause_button((pause_button_t) (PAUSE_BUTTON_COUNT - 1));

  uint16_t title_scale = vres >= 700 ? 6 : 4;
  uint16_t title_height = TEXT_FONT_HEIGHT * title_scale;
  uint16_t padding = vres / 16;

  uint16_t panel_x = first_button.x > padding ? first_button.x - padding : 0;
  uint16_t panel_top = first_button.y;

  if (panel_top > title_height + 2 * padding)
    panel_top -= title_height + 2 * padding;
  else
    panel_top = 0;

  uint16_t panel_w = first_button.width + 2 * padding;

  if ((uint32_t) panel_x + panel_w > hres)
    panel_w = hres - panel_x;

  uint16_t panel_bottom = last_button.y + last_button.height + padding;
  uint16_t panel_h = panel_bottom > panel_top ? panel_bottom - panel_top : 0;

  if (renderer_submit_rect(panel_x, panel_top, panel_w, panel_h, 0x00, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_rect(panel_x + 2, panel_top + 2, panel_w - 4, panel_h - 4, 0x03, RENDER_LAYER_UI + 1) != 0)
    return 1;

  uint16_t title_y = panel_top + padding;

  if (renderer_submit_text("PAUSED", hres / 2 - text_width("PAUSED", title_scale) / 2, title_y, title_scale, 0x0F, RENDER_LAYER_UI + 2) != 0)
    return 1;

  static const char *labels[PAUSE_BUTTON_COUNT] = {"RESUME", "RESTART", "MAIN MENU", "EXIT"};
  pause_button_t selected = pause_state_get_selected();

  for (uint8_t i = 0; i < PAUSE_BUTTON_COUNT; i++) {
    ui_rect_t button = ui_layout_get_pause_button((pause_button_t) i);

    if ((pause_button_t) i == selected) {
      uint16_t outline = 4;
      uint16_t ox = button.x > outline ? button.x - outline : 0;
      uint16_t oy = button.y > outline ? button.y - outline : 0;

      if (renderer_submit_rect(ox, oy, button.width + 2 * outline, button.height + 2 * outline, 0x0E, RENDER_LAYER_UI + 2) != 0)
        return 1;
    }

    if (renderer_submit_button_visual(&button, labels[i], RENDER_LAYER_UI + 3) != 0)
      return 1;
  }

  return 0;
}

int renderer_draw_game_over(void) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  static char final_score_text[24];
  static char best_score_text[24];

  if (renderer_submit_rect(0, 0, hres, vres, 0x04, RENDER_LAYER_BACKGROUND) != 0)
    return 1;

  uint16_t title_scale = vres >= 700 ? 7 : 5;

  if (renderer_submit_text("GAME OVER", hres / 2 - text_width("GAME OVER", title_scale) / 2, vres / 6, title_scale, 0x0F, RENDER_LAYER_UI) != 0)
    return 1;

  snprintf(final_score_text, sizeof(final_score_text), "FINAL SCORE %u", (unsigned int) score_get_value(singleplayer_state_get_final_score()));

  uint16_t score_scale = vres >= 700 ? 4 : 3;
  uint16_t score_y = vres / 6 + TEXT_FONT_HEIGHT * title_scale + vres / 20;

  if (renderer_submit_text(final_score_text, hres / 2 - text_width(final_score_text, score_scale) / 2, score_y, score_scale, 0x0E, RENDER_LAYER_UI) != 0)
    return 1;

  snprintf(best_score_text, sizeof(best_score_text), "HIGHEST SCORE %u", (unsigned int) high_score_get_value());
  if (renderer_submit_text(best_score_text, hres / 2 - text_width(best_score_text, score_scale) / 2, score_y + TEXT_FONT_HEIGHT * score_scale + 12, score_scale, 0x0B, RENDER_LAYER_UI) != 0)
    return 1;

  static const char *labels[GAME_OVER_BUTTON_COUNT] = {"RESTART", "MAIN MENU", "EXIT"};
  game_over_button_t selected = game_over_state_get_selected();

  for (uint8_t i = 0; i < GAME_OVER_BUTTON_COUNT; i++) {
    ui_rect_t button = ui_layout_get_game_over_button((game_over_button_t) i);

    if ((game_over_button_t) i == selected) {
      uint16_t outline = 4;
      uint16_t ox = button.x > outline ? button.x - outline : 0;
      uint16_t oy = button.y > outline ? button.y - outline : 0;

      if (renderer_submit_rect(ox, oy, button.width + 2 * outline, button.height + 2 * outline, 0x0E, RENDER_LAYER_UI + 2) != 0)
        return 1;
    }

    if (renderer_submit_button_visual(&button, labels[i], RENDER_LAYER_UI + 3) != 0)
      return 1;
  }

  return 0;
}
