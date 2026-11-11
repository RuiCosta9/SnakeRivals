/**
 * @file ui_layout.c
 * @brief Shared fixed menu layout rectangle implementation.
 */

#include "ui_layout.h"

#include <stddef.h>

#include "vbe.h"

static uint16_t ui_layout_min_u16(uint16_t lhs, uint16_t rhs) {
  return lhs < rhs ? lhs : rhs;
}

static uint16_t ui_layout_max_u16(uint16_t lhs, uint16_t rhs) {
  return lhs > rhs ? lhs : rhs;
}

bool ui_rect_contains(const ui_rect_t *rect, uint16_t x, uint16_t y) {
  if (rect == NULL)
    return false; 

  return x >= rect->x && x < rect->x + rect->width && y >= rect->y && y < rect->y + rect->height;
}

ui_rect_t ui_layout_get_menu_button(menu_button_t button) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  uint16_t width = ui_layout_max_u16(hres / 3, 180);
  uint16_t height = ui_layout_max_u16(vres / 12, 40);
  uint16_t gap = ui_layout_max_u16(vres / 24, 18);
  uint16_t total_height = (uint16_t) MENU_BUTTON_COUNT * height + (uint16_t) (MENU_BUTTON_COUNT - 1) * gap;
  uint16_t start_y = vres / 3;

  if (hres > 40)
    width = ui_layout_min_u16(width, hres - 40);
  else
    width = hres;

  if (vres > 40)
    height = ui_layout_min_u16(height, vres / 5);

  total_height = (uint16_t) MENU_BUTTON_COUNT * height + (uint16_t) (MENU_BUTTON_COUNT - 1) * gap;

  if ((uint32_t) start_y + total_height > vres && vres > total_height)
    start_y = (vres - total_height) / 2;

  ui_rect_t rect;
  rect.x = (hres > width) ? (hres - width) / 2 : 0;
  rect.y = start_y + (uint16_t) button * (height + gap);
  rect.width = width;
  rect.height = height;

  return rect;
}

ui_rect_t ui_layout_get_play_menu_button(play_menu_button_t button) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  uint16_t width = ui_layout_max_u16(hres / 3, 180);
  uint16_t height = ui_layout_max_u16(vres / 12, 40);
  uint16_t gap = ui_layout_max_u16(vres / 24, 18);
  uint16_t total_height = (uint16_t) PLAY_MENU_BUTTON_COUNT * height + (uint16_t) (PLAY_MENU_BUTTON_COUNT - 1) * gap;
  uint16_t start_y = vres / 3;

  if (hres > 40)
    width = ui_layout_min_u16(width, hres - 40);
  else
    width = hres;

  if (vres > 40)
    height = ui_layout_min_u16(height, vres / 5);

  total_height = (uint16_t) PLAY_MENU_BUTTON_COUNT * height + (uint16_t) (PLAY_MENU_BUTTON_COUNT - 1) * gap;

  if ((uint32_t) start_y + total_height > vres && vres > total_height)
    start_y = (vres - total_height) / 2;

  ui_rect_t rect;
  rect.x = (hres > width) ? (hres - width) / 2 : 0;
  rect.y = start_y + (uint16_t) button * (height + gap);
  rect.width = width;
  rect.height = height;

  return rect;
}

ui_rect_t ui_layout_get_pause_button(pause_button_t button) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  uint16_t width = ui_layout_max_u16(hres / 3, 180);
  uint16_t height = ui_layout_max_u16(vres / 14, 36);
  uint16_t gap = ui_layout_max_u16(vres / 32, 14);

  if (hres > 40)
    width = ui_layout_min_u16(width, hres - 40);
  else
    width = hres;

  if (vres > 40)
    height = ui_layout_min_u16(height, vres / 7);

  uint16_t total_height = (uint16_t) PAUSE_BUTTON_COUNT * height + (uint16_t) (PAUSE_BUTTON_COUNT - 1) * gap;

  uint16_t start_y = vres / 2 - total_height / 2 + vres / 16;

  if ((uint32_t) start_y + total_height > vres && vres > total_height)
    start_y = (vres - total_height) / 2;

  ui_rect_t rect;
  rect.x = (hres > width) ? (hres - width) / 2 : 0;
  rect.y = start_y + (uint16_t) button * (height + gap);
  rect.width = width;
  rect.height = height;

  return rect;
}

ui_rect_t ui_layout_get_game_over_button(game_over_button_t button) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  uint16_t width = ui_layout_max_u16(hres / 3, 180);
  uint16_t height = ui_layout_max_u16(vres / 14, 36);
  uint16_t gap = ui_layout_max_u16(vres / 32, 14);

  if (hres > 40)
    width = ui_layout_min_u16(width, hres - 40);
  else
    width = hres;

  if (vres > 40)
    height = ui_layout_min_u16(height, vres / 7);

  uint16_t total_height = (uint16_t) GAME_OVER_BUTTON_COUNT * height + (uint16_t) (GAME_OVER_BUTTON_COUNT - 1) * gap;

  uint16_t start_y = vres / 2 + vres / 12;

  if ((uint32_t) start_y + total_height > vres)
    start_y = (vres > total_height + vres / 16) ? vres - total_height - vres / 16 : 0;

  ui_rect_t rect;
  rect.x = (hres > width) ? (hres - width) / 2 : 0;
  rect.y = start_y + (uint16_t) button * (height + gap);
  rect.width = width;
  rect.height = height;

  return rect;
}

ui_rect_t ui_layout_get_multiplayer_lobby_button(multiplayer_lobby_button_t button) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();
  uint16_t gap = ui_layout_max_u16(vres / 48, 16);
  uint16_t group_gap = ui_layout_max_u16(vres / 18, 34);
  uint16_t mode_w = ui_layout_max_u16(hres / 5, 170);
  uint16_t mode_h = ui_layout_max_u16(vres / 18, 34);
  uint16_t card_w = ui_layout_max_u16(hres / 5, 190);
  uint16_t card_h = ui_layout_max_u16(vres / 5, 140);
  uint16_t action_w = ui_layout_max_u16(hres / 5, 170);
  uint16_t action_h = ui_layout_max_u16(vres / 17, 36);

  if (hres > 80) {
    mode_w = ui_layout_min_u16(mode_w, (uint16_t) ((hres - 3 * gap) / 2));
    card_w = ui_layout_min_u16(card_w, (uint16_t) ((hres - 3 * gap) / 2));
    action_w = ui_layout_min_u16(action_w, (uint16_t) ((hres - 3 * gap) / 2));
  }

  card_h = ui_layout_min_u16(card_h, ui_layout_max_u16(vres / 4, 150));

  uint16_t mode_total_w = (uint16_t) (2 * mode_w + gap);
  uint16_t card_total_w = (uint16_t) (2 * card_w + gap);
  uint16_t action_total_w = (uint16_t) (2 * action_w + gap);
  uint16_t mode_x = hres > mode_total_w ? (uint16_t) ((hres - mode_total_w) / 2) : 0;
  uint16_t card_x = hres > card_total_w ? (uint16_t) ((hres - card_total_w) / 2) : 0;
  uint16_t action_x = hres > action_total_w ? (uint16_t) ((hres - action_total_w) / 2) : 0;

  /* Keep the lobby in a stable three-block vertical rhythm:
     header/status above, then MODE, COLORS, ACTIONS. The previous centering
     formula could pull MODE upward and make the host/client helper labels
     overlap the group titles in 800x600. */
  uint16_t content_h = (uint16_t) (mode_h + card_h + action_h + 2 * group_gap);
  uint16_t preferred_mode_y = ui_layout_max_u16((uint16_t) (vres / 3 + vres / 12), 230);
  uint16_t max_mode_y = vres > content_h + gap ? (uint16_t) (vres - content_h - gap) : gap;
  uint16_t mode_y = ui_layout_min_u16(preferred_mode_y, max_mode_y);

  if (mode_y < 210 && vres > content_h + 210)
    mode_y = 210;

  uint16_t card_y = (uint16_t) (mode_y + mode_h + group_gap);
  uint16_t action_y = (uint16_t) (card_y + card_h + group_gap);

  ui_rect_t rect;
  rect.x = 0;
  rect.y = 0;
  rect.width = mode_w;
  rect.height = mode_h;

  switch (button) {
    case MULTIPLAYER_LOBBY_BUTTON_SCORE_RACE:
      rect.x = mode_x;
      rect.y = mode_y;
      rect.width = mode_w;
      rect.height = mode_h;
      break;

    case MULTIPLAYER_LOBBY_BUTTON_ARENA_BATTLE:
      rect.x = (uint16_t) (mode_x + mode_w + gap);
      rect.y = mode_y;
      rect.width = mode_w;
      rect.height = mode_h;
      break;

    case MULTIPLAYER_LOBBY_BUTTON_PLAYER1_COLOR:
      rect.x = card_x;
      rect.y = card_y;
      rect.width = card_w;
      rect.height = card_h;
      break;

    case MULTIPLAYER_LOBBY_BUTTON_PLAYER2_COLOR:
      rect.x = (uint16_t) (card_x + card_w + gap);
      rect.y = card_y;
      rect.width = card_w;
      rect.height = card_h;
      break;

    case MULTIPLAYER_LOBBY_BUTTON_START:
      rect.x = action_x;
      rect.y = action_y;
      rect.width = action_w;
      rect.height = action_h;
      break;

    case MULTIPLAYER_LOBBY_BUTTON_BACK:
      rect.x = (uint16_t) (action_x + action_w + gap);
      rect.y = action_y;
      rect.width = action_w;
      rect.height = action_h;
      break;

    default:
      break;
  }

  return rect;
}

ui_rect_t ui_layout_get_instructions_back_button(void) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  uint16_t margin_x = hres / 14;
  uint16_t margin_y = vres / 12;
  uint16_t panel_w = hres - 2 * margin_x;
  uint16_t panel_h = vres - 2 * margin_y;

  uint16_t size = ui_layout_max_u16(vres / 14, 56);
  size = ui_layout_min_u16(size, ui_layout_max_u16(panel_w / 8, 32));
  size = ui_layout_min_u16(size, ui_layout_max_u16(panel_h / 8, 32));

  ui_rect_t rect;
  rect.x = margin_x + panel_w / 32;
  rect.y = margin_y + panel_h / 24;
  rect.width = size;
  rect.height = size;

  return rect;
}

ui_rect_t ui_layout_get_join_button(join_button_t button) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  uint16_t width = ui_layout_max_u16(hres / 4, 160);
  uint16_t height = ui_layout_max_u16(vres / 16, 34);
  uint16_t gap = ui_layout_max_u16(vres / 36, 12);
  uint16_t total_width = (uint16_t) JOIN_BUTTON_COUNT * width + (uint16_t) (JOIN_BUTTON_COUNT - 1) * gap;
  uint16_t start_x = hres > total_width ? (hres - total_width) / 2 : 0;

  ui_rect_t rect;
  rect.x = start_x + (uint16_t) button * (width + gap);
  rect.y = vres / 2 + vres / 7;
  rect.width = width;
  rect.height = height;

  return rect;
}

ui_rect_t ui_layout_get_multiplayer_game_over_button(void) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();
  uint16_t width = ui_layout_max_u16(hres / 3, 220);
  uint16_t height = ui_layout_max_u16(vres / 15, 42);

  if (hres > 40)
    width = ui_layout_min_u16(width, hres - 40);

  ui_rect_t rect;
  rect.x = hres > width ? (hres - width) / 2 : 0;
  rect.y = vres / 2 + vres / 8;
  rect.width = width;
  rect.height = height;
  return rect;
}
