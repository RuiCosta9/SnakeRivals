/**
 * @file menu_state.c
 * @brief Main menu navigation and transition handling.
 */

#include "menu_state.h"

#include "menu_nav.h"
#include "mouse_cursor.h"
#include "state.h"
#include "ui_layout.h"

static uint8_t selected = MENU_BUTTON_PLAY;

static ui_rect_t menu_state_button_rect(uint8_t index) {
  return ui_layout_get_menu_button((menu_button_t) index);
}

static void menu_state_activate(menu_button_t button) {
  switch (button) {
    case MENU_BUTTON_PLAY:
      state_set_current(STATE_PLAY_MENU);
      break;

    case MENU_BUTTON_HOST:
      state_set_current(STATE_MULTIPLAYER_HOST);
      break;

    case MENU_BUTTON_INSTRUCTIONS:
      state_set_current(STATE_INSTRUCTIONS);
      break;

    case MENU_BUTTON_EXIT:
      state_set_current(STATE_EXIT);
      break;

    default:
      break;
  }
}

menu_button_t menu_state_get_selected(void) {
  if (selected >= MENU_BUTTON_COUNT)
    selected = MENU_BUTTON_PLAY;

  return (menu_button_t) selected;
}

void menu_state_handle_timer(void) {
}

void menu_state_handle_keyboard(uint8_t scancode) {
  bool activate = false;

  if (menu_nav_handle_keyboard(scancode, &selected, MENU_BUTTON_COUNT, &activate)) {
    if (activate)
      menu_state_activate((menu_button_t) selected);
    return;
  }

  switch (scancode) {
    case STATE_KEY_P_MAKE:
      selected = MENU_BUTTON_PLAY;
      menu_state_activate(MENU_BUTTON_PLAY);
      break;

    case STATE_KEY_H_MAKE:
      selected = MENU_BUTTON_HOST;
      menu_state_activate(MENU_BUTTON_HOST);
      break;

    case STATE_KEY_I_MAKE:
      selected = MENU_BUTTON_INSTRUCTIONS;
      menu_state_activate(MENU_BUTTON_INSTRUCTIONS);
      break;

    case STATE_KEY_ESC_BREAK:
      selected = MENU_BUTTON_EXIT;
      menu_state_activate(MENU_BUTTON_EXIT);
      break;

    default:
      break;
  }
}

void menu_state_handle_mouse(const struct packet *packet) {
  (void) packet;

  uint8_t activated = 0;
  uint16_t cursor_x = (uint16_t) mouse_cursor_get_x();
  uint16_t cursor_y = (uint16_t) mouse_cursor_get_y();

  if (menu_nav_handle_mouse(menu_state_button_rect,
                            MENU_BUTTON_COUNT,
                            cursor_x,
                            cursor_y,
                            mouse_cursor_was_left_clicked(),
                            &selected,
                            &activated)) {
    menu_state_activate((menu_button_t) activated);
  }
}
