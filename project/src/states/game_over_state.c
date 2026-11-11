/**
 * @file game_over_state.c
 * @brief Game-over menu navigation and final transitions.
 */

#include "game_over_state.h"

#include "mouse_cursor.h"
#include "singleplayer_state.h"
#include "state.h"
#include "ui_layout.h"
#include <stdint.h>

static game_over_button_t selected = GAME_OVER_BUTTON_RESTART;

void game_over_state_reset(void) {
  selected = GAME_OVER_BUTTON_RESTART;
}

game_over_button_t game_over_state_get_selected(void) {
  return selected;
}

static void game_over_state_activate(game_over_button_t option) {
  switch (option) {
    case GAME_OVER_BUTTON_RESTART:
      singleplayer_state_reset();
      state_set_current(STATE_SINGLEPLAYER);
      break;

    case GAME_OVER_BUTTON_MAIN_MENU:
      state_set_current(STATE_MENU);
      break;

    case GAME_OVER_BUTTON_EXIT:
      state_set_current(STATE_EXIT);
      break;

    default:
      break;
  }
}

void game_over_state_handle_timer(void) {
  /*
   * Later, game over screen animations can be updated here.
   */
}

void game_over_state_handle_keyboard(uint8_t scancode) {
  switch (scancode) {
    case STATE_KEY_W_MAKE:
    case STATE_KEY_UP_MAKE:
      selected = (game_over_button_t) ((selected + GAME_OVER_BUTTON_COUNT - 1) % GAME_OVER_BUTTON_COUNT);
      break;

    case STATE_KEY_S_MAKE:
    case STATE_KEY_DOWN_MAKE:
      selected = (game_over_button_t) ((selected + 1) % GAME_OVER_BUTTON_COUNT);
      break;

    case STATE_KEY_ENTER_MAKE:
      game_over_state_activate(selected);
      break;

    case STATE_KEY_ESC_BREAK:
      game_over_state_activate(GAME_OVER_BUTTON_MAIN_MENU);
      break;

    case STATE_KEY_1_MAKE:
      game_over_state_activate(GAME_OVER_BUTTON_RESTART);
      break;

    case STATE_KEY_2_MAKE:
      game_over_state_activate(GAME_OVER_BUTTON_MAIN_MENU);
      break;

    case STATE_KEY_3_MAKE:
      game_over_state_activate(GAME_OVER_BUTTON_EXIT);
      break;

    default:
      break;
  }
}

void game_over_state_handle_mouse(const struct packet *packet) {
  (void) packet;

  uint16_t cursor_x = (uint16_t) mouse_cursor_get_x();
  uint16_t cursor_y = (uint16_t) mouse_cursor_get_y();

  for (uint8_t i = 0; i < GAME_OVER_BUTTON_COUNT; i++) {
    ui_rect_t button = ui_layout_get_game_over_button((game_over_button_t) i);

    if (ui_rect_contains(&button, cursor_x, cursor_y)) {
      selected = (game_over_button_t) i;
      break;
    }
  }

  if (!mouse_cursor_was_left_clicked())
    return;

  for (uint8_t i = 0; i < GAME_OVER_BUTTON_COUNT; i++) {
    ui_rect_t button = ui_layout_get_game_over_button((game_over_button_t) i);

    if (ui_rect_contains(&button, cursor_x, cursor_y)) {
      game_over_state_activate((game_over_button_t) i);
      return;
    }
  }
}
