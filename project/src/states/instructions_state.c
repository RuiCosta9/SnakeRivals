/**
 * @file instructions_state.c
 * @brief Instructions screen input handling.
 */

#include "instructions_state.h"

#include <stdbool.h>

#include "mouse_cursor.h"
#include "state.h"
#include "ui_layout.h"

static bool back_selected = true;

bool instructions_state_is_back_selected(void) {
  return back_selected;
}

void instructions_state_handle_timer(void) {
}

void instructions_state_handle_keyboard(uint8_t scancode) {
  switch (scancode) {
    case STATE_KEY_W_MAKE:
    case STATE_KEY_UP_MAKE:
    case STATE_KEY_S_MAKE:
    case STATE_KEY_DOWN_MAKE:
      back_selected = true;
      break;

    case STATE_KEY_ENTER_MAKE:
    case STATE_KEY_ESC_BREAK:
      state_set_current(STATE_MENU);
      break;

    default:
      break;
  }
}

void instructions_state_handle_mouse(const struct packet *packet) {
  (void) packet;

  uint16_t cursor_x = (uint16_t) mouse_cursor_get_x();
  uint16_t cursor_y = (uint16_t) mouse_cursor_get_y();
  ui_rect_t back_button = ui_layout_get_instructions_back_button();

  if (ui_rect_contains(&back_button, cursor_x, cursor_y))
    back_selected = true;

  if (mouse_cursor_was_left_clicked() && ui_rect_contains(&back_button, cursor_x, cursor_y))
    state_set_current(STATE_MENU);
}
