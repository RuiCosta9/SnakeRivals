/**
 * @file play_menu_state.c
 * @brief Play-menu navigation and mode-selection transitions.
 */

#include "play_menu_state.h"

#include "menu_nav.h"
#include "mouse_cursor.h"
#include "state.h"
#include "ui_layout.h"

static uint8_t selected = PLAY_MENU_BUTTON_SINGLEPLAYER;

static ui_rect_t play_menu_state_button_rect(uint8_t index) {
  return ui_layout_get_play_menu_button((play_menu_button_t) index);
}

static void play_menu_state_activate(play_menu_button_t button) {
  switch (button) {
    case PLAY_MENU_BUTTON_SINGLEPLAYER:
      state_set_current(STATE_SINGLEPLAYER);
      break;

    case PLAY_MENU_BUTTON_JOIN_MULTIPLAYER:
      state_set_current(STATE_MULTIPLAYER_JOIN);
      break;

    case PLAY_MENU_BUTTON_BACK:
      state_set_current(STATE_MENU);
      break;

    default:
      break;
  }
}

play_menu_button_t play_menu_state_get_selected(void) {
  if (selected >= PLAY_MENU_BUTTON_COUNT)
    selected = PLAY_MENU_BUTTON_SINGLEPLAYER;

  return (play_menu_button_t) selected;
}

void play_menu_state_handle_timer(void) {
}

void play_menu_state_handle_keyboard(uint8_t scancode) {
  bool activate = false;

  if (menu_nav_handle_keyboard(scancode, &selected, PLAY_MENU_BUTTON_COUNT, &activate)) {
    if (activate)
      play_menu_state_activate((play_menu_button_t) selected);
    return;
  }

  switch (scancode) {
    case STATE_KEY_S_MAKE:
      selected = PLAY_MENU_BUTTON_SINGLEPLAYER;
      play_menu_state_activate(PLAY_MENU_BUTTON_SINGLEPLAYER);
      break;

    case STATE_KEY_J_MAKE:
    case STATE_KEY_C_MAKE:
      selected = PLAY_MENU_BUTTON_JOIN_MULTIPLAYER;
      play_menu_state_activate(PLAY_MENU_BUTTON_JOIN_MULTIPLAYER);
      break;

    case STATE_KEY_ESC_BREAK:
      selected = PLAY_MENU_BUTTON_BACK;
      play_menu_state_activate(PLAY_MENU_BUTTON_BACK);
      break;

    default:
      break;
  }
}

void play_menu_state_handle_mouse(const struct packet *packet) {
  (void) packet;

  uint8_t activated = 0;
  uint16_t cursor_x = (uint16_t) mouse_cursor_get_x();
  uint16_t cursor_y = (uint16_t) mouse_cursor_get_y();

  if (menu_nav_handle_mouse(play_menu_state_button_rect,
                            PLAY_MENU_BUTTON_COUNT,
                            cursor_x,
                            cursor_y,
                            mouse_cursor_was_left_clicked(),
                            &selected,
                            &activated)) {
    play_menu_state_activate((play_menu_button_t) activated);
  }
}
