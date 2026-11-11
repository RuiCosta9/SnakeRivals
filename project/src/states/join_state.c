/**
 * @file join_state.c
 * @brief Room-code entry and client join transition handling.
 */

#include "join_state.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "menu_nav.h"
#include "mouse_cursor.h"
#include "multiplayer_session.h"
#include "state.h"
#include "ui_layout.h"

#define JOIN_CODE_LENGTH 4

static char code_text[JOIN_CODE_LENGTH + 1] = "";
static uint8_t code_length = 0;
static const char *status_text = "TYPE 4 DIGITS";
static uint8_t selected = JOIN_BUTTON_JOIN;

static ui_rect_t join_state_button_rect(uint8_t index) {
  return ui_layout_get_join_button((join_button_t) index);
}

static bool join_state_digit_from_scancode(uint8_t scancode, char *digit) {
  if (digit == NULL)
    return false;

  switch (scancode) {
    case STATE_KEY_0_MAKE: *digit = '0'; return true;
    case STATE_KEY_1_MAKE: *digit = '1'; return true;
    case STATE_KEY_2_MAKE: *digit = '2'; return true;
    case STATE_KEY_3_MAKE: *digit = '3'; return true;
    case STATE_KEY_4_MAKE: *digit = '4'; return true;
    case STATE_KEY_5_MAKE: *digit = '5'; return true;
    case STATE_KEY_6_MAKE: *digit = '6'; return true;
    case STATE_KEY_7_MAKE: *digit = '7'; return true;
    case STATE_KEY_8_MAKE: *digit = '8'; return true;
    case STATE_KEY_9_MAKE: *digit = '9'; return true;
    default: return false;
  }
}

static uint16_t join_state_code_to_u16(void) {
  uint16_t code = 0;

  for (uint8_t i = 0; i < code_length; i++)
    code = (uint16_t) (code * 10 + (uint16_t) (code_text[i] - '0'));

  return code;
}

static void join_state_try_join(void) {
  if (code_length != JOIN_CODE_LENGTH) {
    status_text = "CODE NEEDS 4 DIGITS";
    selected = JOIN_BUTTON_JOIN;
    return;
  }

  multiplayer_session_set_pending_join_code(join_state_code_to_u16());
  state_set_current(STATE_MULTIPLAYER_CLIENT);
}

static void join_state_activate(join_button_t button) {
  selected = button;

  switch (button) {
    case JOIN_BUTTON_JOIN:
      join_state_try_join();
      break;

    case JOIN_BUTTON_BACK:
      state_set_current(STATE_PLAY_MENU);
      break;

    default:
      break;
  }
}

void join_state_reset(void) {
  code_text[0] = '\0';
  code_length = 0;
  status_text = "TYPE 4 DIGITS";
  selected = JOIN_BUTTON_JOIN;
}

join_button_t join_state_get_selected(void) {
  if (selected >= JOIN_BUTTON_COUNT)
    selected = JOIN_BUTTON_JOIN;

  return (join_button_t) selected;
}

void join_state_handle_timer(void) {
}

void join_state_handle_keyboard(uint8_t scancode) {
  char digit;
  bool activate = false;

  if (scancode == STATE_KEY_ESC_BREAK) {
    join_state_activate(JOIN_BUTTON_BACK);
    return;
  }

  if (menu_nav_handle_keyboard(scancode, &selected, JOIN_BUTTON_COUNT, &activate)) {
    if (activate)
      join_state_activate((join_button_t) selected);
    return;
  }

  if (scancode == STATE_KEY_BACKSPACE_MAKE) {
    if (code_length > 0) {
      code_length--;
      code_text[code_length] = '\0';
      status_text = "TYPE 4 DIGITS";
    }
    return;
  }

  if (join_state_digit_from_scancode(scancode, &digit)) {
    if (code_length < JOIN_CODE_LENGTH) {
      code_text[code_length++] = digit;
      code_text[code_length] = '\0';
      status_text = code_length == JOIN_CODE_LENGTH ? "ENTER TO JOIN" : "TYPE 4 DIGITS";
      selected = JOIN_BUTTON_JOIN;
    }
  }
}

void join_state_handle_mouse(const struct packet *packet) {
  (void) packet;

  uint8_t activated = 0;
  uint16_t cursor_x = (uint16_t) mouse_cursor_get_x();
  uint16_t cursor_y = (uint16_t) mouse_cursor_get_y();

  if (menu_nav_handle_mouse(join_state_button_rect,
                            JOIN_BUTTON_COUNT,
                            cursor_x,
                            cursor_y,
                            mouse_cursor_was_left_clicked(),
                            &selected,
                            &activated)) {
    join_state_activate((join_button_t) activated);
  }
}

const char *join_state_get_code_text(void) {
  return code_text;
}

const char *join_state_get_status_text(void) {
  return status_text;
}
