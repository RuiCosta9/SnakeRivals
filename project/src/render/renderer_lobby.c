/**
 * @file renderer_lobby.c
 * @brief Host and client multiplayer lobby rendering.
 */

#include "renderer_internal.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "assets.h"
#include "multiplayer_session.h"
#include "multiplayer_state.h"
#include "sprite.h"
#include "text.h"
#include "ui_layout.h"

static uint16_t renderer_lobby_min_u16(uint16_t lhs, uint16_t rhs) {
  return lhs < rhs ? lhs : rhs;
}

static int renderer_submit_lobby_panel(uint16_t hres, uint16_t vres) {
  uint16_t panel_w = hres > 120 ? (uint16_t) (hres - 120) : hres;
  uint16_t panel_h = vres > 110 ? (uint16_t) (vres - 110) : vres;
  uint16_t panel_x = (uint16_t) ((hres - panel_w) / 2);
  uint16_t panel_y = (uint16_t) ((vres - panel_h) / 2);

  if (renderer_submit_rect(0, 0, hres, vres, 0x01, RENDER_LAYER_BACKGROUND) != 0)
    return 1;

  if (hres > 16 && vres > 16) {
    if (renderer_submit_rect(8, 8, (uint16_t) (hres - 16), (uint16_t) (vres - 16), 0x00, RENDER_LAYER_BACKGROUND + 1) != 0)
      return 1;
  }

  if (renderer_submit_rect(panel_x, panel_y, panel_w, panel_h, 0x00, RENDER_LAYER_UI - 8) != 0)
    return 1;

  if (panel_w > 16 && panel_h > 16) {
    if (renderer_submit_rect((uint16_t) (panel_x + 8), (uint16_t) (panel_y + 8), (uint16_t) (panel_w - 16), (uint16_t) (panel_h - 16), 0x02, RENDER_LAYER_UI - 7) != 0)
      return 1;
  }

  if (panel_w > 32 && panel_h > 32) {
    if (renderer_submit_rect((uint16_t) (panel_x + 16), (uint16_t) (panel_y + 16), (uint16_t) (panel_w - 32), (uint16_t) (panel_h - 32), 0x01, RENDER_LAYER_UI - 6) != 0)
      return 1;
  }

  return 0;
}

static int renderer_submit_lobby_centered_text(const char *text, uint16_t hres, uint16_t y, uint8_t scale, uint32_t color) {
  return renderer_submit_text(text,
                              (uint16_t) (hres / 2 - text_width(text, scale) / 2),
                              y,
                              scale,
                              color,
                              RENDER_LAYER_UI);
}

static int renderer_submit_lobby_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, int16_t z) {
  if (w == 0 || h == 0)
    return 0;

  if (renderer_submit_rect((uint16_t) (x + 4), (uint16_t) (y + 4), w, h, 0x00, z) != 0)
    return 1;

  if (renderer_submit_rect(x, y, w, h, 0x08, z + 1) != 0)
    return 1;

  if (w > 8 && h > 8) {
    if (renderer_submit_rect((uint16_t) (x + 4), (uint16_t) (y + 4), (uint16_t) (w - 8), (uint16_t) (h - 8), 0x00, z + 2) != 0)
      return 1;
  }

  return 0;
}

static int renderer_submit_lobby_button_group(ui_rect_t first, ui_rect_t last, const char *title, uint16_t hres) {
  const uint16_t pad_x = 22;
  const uint16_t pad_top = 30;
  const uint16_t pad_bottom = 16;
  uint16_t x = first.x < last.x ? first.x : last.x;
  uint16_t y = first.y < last.y ? first.y : last.y;
  uint16_t first_right = (uint16_t) (first.x + first.width);
  uint16_t last_right = (uint16_t) (last.x + last.width);
  uint16_t first_bottom = (uint16_t) (first.y + first.height);
  uint16_t last_bottom = (uint16_t) (last.y + last.height);
  uint16_t right = first_right > last_right ? first_right : last_right;
  uint16_t bottom = first_bottom > last_bottom ? first_bottom : last_bottom;

  x = x > pad_x ? (uint16_t) (x - pad_x) : 0;
  y = y > pad_top ? (uint16_t) (y - pad_top) : 0;
  right = (uint16_t) (right + pad_x);
  bottom = (uint16_t) (bottom + pad_bottom);

  if (right > hres)
    right = hres;

  uint16_t w = right > x ? (uint16_t) (right - x) : first.width;
  uint16_t h = bottom > y ? (uint16_t) (bottom - y) : first.height;

  if (renderer_submit_lobby_box(x, y, w, h, RENDER_LAYER_UI - 5) != 0)
    return 1;

  if (title != NULL) {
    if (renderer_submit_text(title,
                             (uint16_t) (x + w / 2 - text_width(title, 2) / 2),
                             (uint16_t) (y + 8),
                             2,
                             0x0E,
                             RENDER_LAYER_UI - 1) != 0)
      return 1;
  }

  return 0;
}

static int renderer_submit_lobby_status_card(uint16_t hres, uint16_t y, const char *status_text, uint32_t color) {
  uint16_t card_w = renderer_lobby_min_u16(hres > 72 ? (uint16_t) (hres - 72) : hres, 430);
  uint16_t card_h = 42;
  uint16_t card_x = hres > card_w ? (uint16_t) ((hres - card_w) / 2) : 0;

  if (renderer_submit_lobby_box(card_x, y, card_w, card_h, RENDER_LAYER_UI - 4) != 0)
    return 1;

  return renderer_submit_text(status_text,
                              (uint16_t) (hres / 2 - text_width(status_text, 2) / 2),
                              (uint16_t) (y + 13),
                              2,
                              color,
                              RENDER_LAYER_UI + 1);
}

static int renderer_submit_lobby_arrow_icon(uint16_t x, uint16_t y, uint16_t size, bool left, uint32_t color, int16_t z) {
  uint16_t unit = size / 7;

  if (unit < 3)
    unit = 3;

  uint16_t center_y = (uint16_t) (y + size / 2);
  uint16_t shaft_h = unit;
  uint16_t shaft_y = (uint16_t) (center_y - shaft_h / 2);
  uint16_t shaft_x = left ? (uint16_t) (x + 3 * unit) : (uint16_t) (x + unit);
  uint16_t shaft_w = size > 4 * unit ? (uint16_t) (size - 4 * unit) : unit;

  if (renderer_submit_rect(shaft_x, shaft_y, shaft_w, shaft_h, color, z) != 0)
    return 1;

  for (uint8_t row = 0; row < 5; row++) {
    uint8_t distance = row > 2 ? (uint8_t) (row - 2) : (uint8_t) (2 - row);
    uint16_t width = (uint16_t) (3 - distance) * unit;
    uint16_t row_y = (uint16_t) (center_y + ((int16_t) row - 2) * (int16_t) unit - (int16_t) unit / 2);
    uint16_t row_x;

    if (left)
      row_x = (uint16_t) (x + unit);
    else
      row_x = (uint16_t) (x + size - unit - width);

    if (renderer_submit_rect(row_x, row_y, width, unit, color, z) != 0)
      return 1;
  }

  return 0;
}


static int renderer_submit_vertical_snake_preview(uint8_t skin, uint16_t center_x, uint16_t y, int16_t z) {
  const sprite_t *head = assets_get_snake_head_for_skin_direction(skin, DIRECTION_UP);
  const sprite_t *body = assets_get_snake_body_for_skin(skin);
  const sprite_t *tail = assets_get_snake_tail_for_skin_direction(skin, DIRECTION_DOWN);
  uint16_t cell = 16;
  uint16_t x = center_x > cell / 2 ? (uint16_t) (center_x - cell / 2) : 0;

  if (head != NULL) {
    if (renderer_submit_sprite(head, x, y, z + 4) != 0)
      return 1;
  }
  else if (renderer_submit_rect(x, y, cell, cell, assets_get_snake_skin_preview_color(skin), z + 4) != 0)
    return 1;

  for (uint8_t i = 1; i < 3; i++) {
    if (body != NULL) {
      if (renderer_submit_sprite(body, x, (uint16_t) (y + i * cell), z + 3 + i) != 0)
        return 1;
    }
    else if (renderer_submit_rect(x, (uint16_t) (y + i * cell), cell, cell, assets_get_snake_skin_preview_color(skin), z + 3 + i) != 0)
      return 1;
  }

  if (tail != NULL)
    return renderer_submit_sprite(tail, x, (uint16_t) (y + 3 * cell), z + 8);

  return renderer_submit_rect(x, (uint16_t) (y + 3 * cell), cell, cell, assets_get_snake_skin_preview_color(skin), z + 8);
}

static int renderer_submit_skin_picker(uint8_t player_id, const ui_rect_t *rect, bool selected, bool enabled) {
  if (rect == NULL)
    return 0;

  char title[12];
  uint8_t skin = multiplayer_session_get_player_skin(player_id);
  uint32_t title_color = enabled ? 0x0F : 0x08;
  uint32_t arrow_color = enabled ? 0x0E : 0x08;

  snprintf(title, sizeof(title), "P%u COLOR", (unsigned) player_id);

  if (selected) {
    if (renderer_submit_rect(rect->x > 4 ? (uint16_t) (rect->x - 4) : 0,
                             rect->y > 4 ? (uint16_t) (rect->y - 4) : 0,
                             (uint16_t) (rect->width + 8),
                             (uint16_t) (rect->height + 8),
                             0x0E,
                             RENDER_LAYER_UI - 1) != 0)
      return 1;
  }

  if (renderer_submit_lobby_box(rect->x, rect->y, rect->width, rect->height, RENDER_LAYER_UI - 2) != 0)
    return 1;

  if (renderer_submit_centered_text(title, (uint16_t) (rect->x + rect->width / 2), (uint16_t) (rect->y + 10), 2, title_color, RENDER_LAYER_UI + 1) != 0)
    return 1;

  if (renderer_submit_vertical_snake_preview(skin, (uint16_t) (rect->x + rect->width / 2), (uint16_t) (rect->y + 42), RENDER_LAYER_UI + 2) != 0)
    return 1;

  if (enabled) {
    uint16_t arrow_size = 28;
    uint16_t arrow_y = (uint16_t) (rect->y + rect->height - 42);
    uint16_t name_y = (uint16_t) (rect->y + rect->height - 32);

    if (renderer_submit_lobby_arrow_icon((uint16_t) (rect->x + rect->width / 2 - 78), arrow_y, arrow_size, true, arrow_color, RENDER_LAYER_UI + 2) != 0)
      return 1;

    if (renderer_submit_centered_text(assets_get_snake_skin_name(skin), (uint16_t) (rect->x + rect->width / 2), name_y, 2, title_color, RENDER_LAYER_UI + 1) != 0)
      return 1;

    if (renderer_submit_lobby_arrow_icon((uint16_t) (rect->x + rect->width / 2 + 50), arrow_y, arrow_size, false, arrow_color, RENDER_LAYER_UI + 2) != 0)
      return 1;
  }
  else {
    if (renderer_submit_centered_text(assets_get_snake_skin_name(skin), (uint16_t) (rect->x + rect->width / 2), (uint16_t) (rect->y + rect->height - 36), 2, title_color, RENDER_LAYER_UI + 1) != 0)
      return 1;

    if (renderer_submit_centered_text("REMOTE", (uint16_t) (rect->x + rect->width / 2), (uint16_t) (rect->y + rect->height - 18), 1, 0x08, RENDER_LAYER_UI + 1) != 0)
      return 1;
  }

  return 0;
}

int renderer_draw_host_lobby(uint16_t hres, uint16_t vres) {
  char code_text[20];
  const bool connected = multiplayer_session_get_connection_state() == MP_CONNECTION_CONNECTED;
  const char *start_label = connected ? "START GAME" : "WAIT PLAYER";
  const char *status_label = connected ? "PLAYER 2 CONNECTED" : "WAITING FOR PLAYER 2";
  multiplayer_lobby_button_t selected_button = multiplayer_state_get_selected_lobby_button();
  multiplayer_game_mode_t current_mode = multiplayer_session_get_game_mode();

  ui_rect_t score_button = ui_layout_get_multiplayer_lobby_button(MULTIPLAYER_LOBBY_BUTTON_SCORE_RACE);
  ui_rect_t arena_button = ui_layout_get_multiplayer_lobby_button(MULTIPLAYER_LOBBY_BUTTON_ARENA_BATTLE);
  ui_rect_t player1_color_button = ui_layout_get_multiplayer_lobby_button(MULTIPLAYER_LOBBY_BUTTON_PLAYER1_COLOR);
  ui_rect_t player2_color_button = ui_layout_get_multiplayer_lobby_button(MULTIPLAYER_LOBBY_BUTTON_PLAYER2_COLOR);
  ui_rect_t start_button = ui_layout_get_multiplayer_lobby_button(MULTIPLAYER_LOBBY_BUTTON_START);
  ui_rect_t back_button = ui_layout_get_multiplayer_lobby_button(MULTIPLAYER_LOBBY_BUTTON_BACK);

  snprintf(code_text, sizeof(code_text), "CODE: %04u", multiplayer_session_get_room_code());
  if (renderer_submit_lobby_panel(hres, vres) != 0)
    return 1;

  uint16_t panel_h = vres > 110 ? (uint16_t) (vres - 110) : vres;
  uint16_t panel_y = (uint16_t) ((vres - panel_h) / 2);
  uint16_t blue_y = (uint16_t) (panel_y + 16);
  uint16_t title_y = (uint16_t) (blue_y + 6);
  uint16_t code_y = (uint16_t) (title_y + 62);
  uint16_t status_y = (uint16_t) (code_y + 56);

  if (renderer_submit_lobby_centered_text("HOST LOBBY", hres, title_y, 5, 0x0F) != 0)
    return 1;

  if (renderer_submit_lobby_centered_text(code_text, hres, code_y, 5, 0x0E) != 0)
    return 1;

  if (renderer_submit_lobby_status_card(hres, status_y, status_label, connected ? 0x0A : 0x0E) != 0)
    return 1;

  if (renderer_submit_lobby_button_group(score_button, arena_button, "MODE", hres) != 0)
    return 1;

  if (renderer_submit_lobby_button_group(player1_color_button, player2_color_button, "SNAKE COLORS", hres) != 0)
    return 1;

  if (renderer_submit_lobby_button_group(start_button, back_button, "ACTIONS", hres) != 0)
    return 1;

  if (renderer_submit_button_visual_state(&score_button, "SCORE RACE", current_mode == MP_MODE_SCORE_RACE || selected_button == MULTIPLAYER_LOBBY_BUTTON_SCORE_RACE, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_button_visual_state(&arena_button, "ARENA BATTLE", current_mode == MP_MODE_ARENA_BATTLE || selected_button == MULTIPLAYER_LOBBY_BUTTON_ARENA_BATTLE, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_skin_picker(1, &player1_color_button, selected_button == MULTIPLAYER_LOBBY_BUTTON_PLAYER1_COLOR, true) != 0)
    return 1;

  if (renderer_submit_skin_picker(2, &player2_color_button, false, false) != 0)
    return 1;

  if (renderer_submit_button_visual_state(&start_button, start_label, selected_button == MULTIPLAYER_LOBBY_BUTTON_START && connected, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_button_visual_state(&back_button, "BACK", selected_button == MULTIPLAYER_LOBBY_BUTTON_BACK, RENDER_LAYER_UI) != 0)
    return 1;

  return 0;
}

int renderer_draw_client_lobby(uint16_t hres, uint16_t vres) {
  char mode_text[40];
  ui_rect_t player1_color_button = ui_layout_get_multiplayer_lobby_button(MULTIPLAYER_LOBBY_BUTTON_PLAYER1_COLOR);
  ui_rect_t player2_color_button = ui_layout_get_multiplayer_lobby_button(MULTIPLAYER_LOBBY_BUTTON_PLAYER2_COLOR);
  ui_rect_t back_button = ui_layout_get_multiplayer_lobby_button(MULTIPLAYER_LOBBY_BUTTON_BACK);
  multiplayer_lobby_button_t selected_button = multiplayer_state_get_selected_lobby_button();

  snprintf(mode_text, sizeof(mode_text), "MODE: %s", multiplayer_session_game_mode_to_string(multiplayer_session_get_game_mode()));

  if (renderer_submit_lobby_panel(hres, vres) != 0)
    return 1;

  if (renderer_submit_lobby_centered_text("JOINED LOBBY", hres, vres / 8, 5, 0x0F) != 0)
    return 1;

  if (renderer_submit_lobby_status_card(hres, (uint16_t) (vres / 4), "CONNECTED TO HOST", 0x0A) != 0)
    return 1;

  if (renderer_submit_lobby_centered_text("WAITING FOR LEADER", hres, player1_color_button.y > 86 ? (uint16_t) (player1_color_button.y - 86) : (uint16_t) (vres / 2 - 112), 4, 0x0E) != 0)
    return 1;

  if (renderer_submit_lobby_centered_text(mode_text, hres, player1_color_button.y > 50 ? (uint16_t) (player1_color_button.y - 50) : (uint16_t) (vres / 2 - 62), 2, 0x0F) != 0)
    return 1;

  if (renderer_submit_lobby_button_group(player1_color_button, player2_color_button, "SNAKE COLORS", hres) != 0)
    return 1;

  if (renderer_submit_lobby_button_group(back_button, back_button, "ACTIONS", hres) != 0)
    return 1;

  if (renderer_submit_skin_picker(1, &player1_color_button, false, false) != 0)
    return 1;

  if (renderer_submit_skin_picker(2, &player2_color_button, selected_button == MULTIPLAYER_LOBBY_BUTTON_PLAYER2_COLOR, true) != 0)
    return 1;

  if (renderer_submit_button_visual_state(&back_button, "BACK", selected_button == MULTIPLAYER_LOBBY_BUTTON_BACK, RENDER_LAYER_UI) != 0)
    return 1;

  return 0;
}
