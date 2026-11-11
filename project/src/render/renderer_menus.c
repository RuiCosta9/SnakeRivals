/**
 * @file renderer_menus.c
 * @brief Rendering for menu, play-menu, join and instructions screens.
 */

#include "renderer_internal.h"
#include "renderer.h"

#include <stdbool.h>
#include <stdint.h>

#include "assets.h"
#include "instructions_state.h"
#include "join_state.h"
#include "menu_state.h"
#include "play_menu_state.h"
#include "text.h"
#include "ui_layout.h"
#include "vbe.h"

static int renderer_submit_title(const char *title, uint16_t hres, uint16_t y, uint8_t scale) {
  return renderer_submit_text(title, hres / 2 - text_width(title, scale) / 2, y, scale, 0x0F, RENDER_LAYER_UI);
}

static int renderer_submit_home_logo(uint16_t hres, uint16_t vres) {
  const sprite_t *logo = assets_get_logo();

  if (logo == NULL)
    return 0;

  uint16_t logo_w = sprite_get_width(logo);
  uint16_t logo_h = sprite_get_height(logo);
  uint16_t logo_x = hres > logo_w ? (hres - logo_w) / 2 : 0;
  uint16_t logo_y = vres / 12;

  if (logo_h > 0 && logo_y + logo_h > vres / 3)
    logo_y = vres / 18;

  return renderer_submit_sprite(logo, logo_x, logo_y, RENDER_LAYER_UI);
}

static int renderer_submit_menu_background(uint16_t hres, uint16_t vres) {
  if (renderer_submit_rect(0, 0, hres, vres, 0x01, RENDER_LAYER_BACKGROUND) != 0)
    return 1;

  if (renderer_submit_rect(hres / 16, vres / 18, hres - hres / 8, 4, 0x08, RENDER_LAYER_BACKGROUND + 1) != 0)
    return 1;

  if (renderer_submit_rect(hres / 16, vres - vres / 16, hres - hres / 8, 4, 0x08, RENDER_LAYER_BACKGROUND + 1) != 0)
    return 1;

  return 0;
}

int renderer_draw_menu(void) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();
  menu_button_t selected = menu_state_get_selected();

  static const char *labels[MENU_BUTTON_COUNT] = {
    "PLAY",
    "HOST",
    "INSTRUCTIONS",
    "EXIT"
  };

  if (renderer_submit_menu_background(hres, vres) != 0)
    return 1;

  if (renderer_submit_home_logo(hres, vres) != 0)
    return 1;

  if (renderer_submit_centered_text("MULTIPLAYER SNAKE ARENA", hres / 2, vres / 8 + 118, 2, 0x0B, RENDER_LAYER_UI) != 0)
    return 1;

  for (uint8_t i = 0; i < MENU_BUTTON_COUNT; i++) {
    ui_rect_t button = ui_layout_get_menu_button((menu_button_t) i);

    if (renderer_submit_button_visual_state(&button, labels[i], (menu_button_t) i == selected, RENDER_LAYER_UI) != 0)
      return 1;
  }

  return 0;
}

int renderer_draw_play_menu(void) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();
  play_menu_button_t selected = play_menu_state_get_selected();

  static const char *labels[PLAY_MENU_BUTTON_COUNT] = {
    "SINGLEPLAYER",
    "JOIN MULTIPLAYER",
    "BACK"
  };

  if (renderer_submit_menu_background(hres, vres) != 0)
    return 1;

  if (renderer_submit_title("PLAY", hres, vres / 8, vres >= 700 ? 7 : 5) != 0)
    return 1;

  for (uint8_t i = 0; i < PLAY_MENU_BUTTON_COUNT; i++) {
    ui_rect_t button = ui_layout_get_play_menu_button((play_menu_button_t) i);

    if (renderer_submit_button_visual_state(&button, labels[i], (play_menu_button_t) i == selected, RENDER_LAYER_UI) != 0)
      return 1;
  }

  return 0;
}

int renderer_draw_join_menu(void) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  char input_text[16];
  const char *code = join_state_get_code_text();
  uint8_t code_len = 0;
  join_button_t selected = join_state_get_selected();

  static const char *labels[JOIN_BUTTON_COUNT] = {
    "JOIN",
    "BACK"
  };

  while (code[code_len] != '\0' && code_len < 4)
    code_len++;

  for (uint8_t i = 0; i < 4; i++)
    input_text[i] = i < code_len ? code[i] : '-';

  input_text[4] = '\0';

  if (renderer_submit_menu_background(hres, vres) != 0)
    return 1;

  if (renderer_submit_title("JOIN MULTIPLAYER", hres, vres / 7, 5) != 0)
    return 1;

  if (renderer_submit_centered_text("ENTER ROOM CODE", hres / 2, vres / 3, 3, 0x0E, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_rect(hres / 2 - 120, vres / 2 - 34, 240, 72, 0x08, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_rect(hres / 2 - 112, vres / 2 - 26, 224, 56, 0x00, RENDER_LAYER_UI + 1) != 0)
    return 1;

  if (renderer_submit_text(input_text, hres / 2 - text_width(input_text, 7) / 2, vres / 2 - 22, 7, 0x0F, RENDER_LAYER_UI + 2) != 0)
    return 1;

  if (renderer_submit_centered_text(join_state_get_status_text(), hres / 2, vres / 2 + 56, 2, 0x0B, RENDER_LAYER_UI) != 0)
    return 1;

  for (uint8_t i = 0; i < JOIN_BUTTON_COUNT; i++) {
    ui_rect_t button = ui_layout_get_join_button((join_button_t) i);

    if (renderer_submit_button_visual_state(&button, labels[i], (join_button_t) i == selected, RENDER_LAYER_UI) != 0)
      return 1;
  }

  return 0;
}

int renderer_draw_instructions(void) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  uint16_t margin_x = hres / 14;
  uint16_t margin_y = vres / 12;
  uint16_t panel_x = margin_x;
  uint16_t panel_y = margin_y;
  uint16_t panel_w = hres - 2 * margin_x;
  uint16_t panel_h = vres - 2 * margin_y;
  ui_rect_t back_button = ui_layout_get_instructions_back_button();

  uint8_t title_scale = vres >= 700 ? 6 : 5;
  uint8_t heading_scale = vres >= 700 ? 4 : 3;
  uint8_t text_scale = vres >= 700 ? 3 : 2;
  uint16_t text_x = panel_x + panel_w / 10;
  uint16_t y = panel_y + panel_h / 4;
  uint16_t line_gap = TEXT_FONT_HEIGHT * text_scale + 14;

  if (renderer_submit_rect(0, 0, hres, vres, 0x01, RENDER_LAYER_BACKGROUND) != 0)
    return 1;

  if (renderer_submit_rect(panel_x, panel_y, panel_w, panel_h, 0x08, RENDER_LAYER_BACKGROUND + 1) != 0)
    return 1;

  if (renderer_submit_rect(panel_x + 6, panel_y + 6, panel_w - 12, panel_h - 12, 0x00, RENDER_LAYER_BACKGROUND + 2) != 0)
    return 1;

  if (instructions_state_is_back_selected()) {
    if (renderer_submit_rect(back_button.x > 4 ? back_button.x - 4 : 0, back_button.y > 4 ? back_button.y - 4 : 0, back_button.width + 8, back_button.height + 8, 0x0E, RENDER_LAYER_UI - 1) != 0)
      return 1;
  }

  if (renderer_submit_back_arrow(back_button.x, back_button.y, back_button.width, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_title("INSTRUCTIONS", hres, panel_y + panel_h / 13, title_scale) != 0)
    return 1;

  if (renderer_submit_text("CONTROLS", text_x, y, heading_scale, 0x0E, RENDER_LAYER_UI) != 0)
    return 1;

  y += line_gap + 6;

  if (renderer_submit_text("MOVE: WASD OR ARROW KEYS", text_x, y, text_scale, 0x0F, RENDER_LAYER_UI) != 0)
    return 1;

  y += line_gap;

  if (renderer_submit_text("POWERUP: SPACE OR LEFT CLICK", text_x, y, text_scale, 0x0F, RENDER_LAYER_UI) != 0)
    return 1;

  y += line_gap;

  if (renderer_submit_text("PAUSE: P OR ESC DURING GAME", text_x, y, text_scale, 0x0F, RENDER_LAYER_UI) != 0)
    return 1;

  y += line_gap + 8;

  if (renderer_submit_text("MULTIPLAYER", text_x, y, heading_scale, 0x0E, RENDER_LAYER_UI) != 0)
    return 1;

  y += line_gap + 6;

  if (renderer_submit_text("SCORE RACE: BEST SCORE WINS", text_x, y, text_scale, 0x0F, RENDER_LAYER_UI) != 0)
    return 1;

  y += line_gap;

  if (renderer_submit_text("ARENA BATTLE: LAST SNAKE ALIVE", text_x, y, text_scale, 0x0F, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_centered_text("ENTER OR CLICK ARROW TO GO BACK", hres / 2, panel_y + panel_h - panel_h / 9, 2, 0x0B, RENDER_LAYER_UI) != 0)
    return 1;

  return 0;
}
