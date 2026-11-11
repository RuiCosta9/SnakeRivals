/**
 * @file multiplayer_state.c
 * @brief UI/input bridge for lobby and multiplayer gameplay events.
 */

#include "multiplayer_state.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mouse_cursor.h"
#include "state.h"
#include "ui_layout.h"

static multiplayer_lobby_button_t selected_lobby_button = MULTIPLAYER_LOBBY_BUTTON_SCORE_RACE;

static bool multiplayer_state_scancode_to_direction(uint8_t scancode, direction_t *direction) {
  if (direction == NULL)
    return false;

  switch (scancode) {
    case STATE_KEY_W_MAKE:
    case STATE_KEY_UP_MAKE:
      *direction = DIRECTION_UP;
      return true;

    case STATE_KEY_D_MAKE:
    case STATE_KEY_RIGHT_MAKE:
      *direction = DIRECTION_RIGHT;
      return true;

    case STATE_KEY_S_MAKE:
    case STATE_KEY_DOWN_MAKE:
      *direction = DIRECTION_DOWN;
      return true;

    case STATE_KEY_A_MAKE:
    case STATE_KEY_LEFT_MAKE:
      *direction = DIRECTION_LEFT;
      return true;

    default:
      return false;
  }
}

static bool multiplayer_state_lobby_button_is_selectable(multiplayer_lobby_button_t button) {
  multiplayer_role_t role = multiplayer_session_get_role();

  if (role == MP_ROLE_HOST) {
    return button == MULTIPLAYER_LOBBY_BUTTON_SCORE_RACE ||
           button == MULTIPLAYER_LOBBY_BUTTON_ARENA_BATTLE ||
           button == MULTIPLAYER_LOBBY_BUTTON_PLAYER1_COLOR ||
           button == MULTIPLAYER_LOBBY_BUTTON_START ||
           button == MULTIPLAYER_LOBBY_BUTTON_BACK;
  }

  if (role == MP_ROLE_CLIENT)
    return button == MULTIPLAYER_LOBBY_BUTTON_PLAYER2_COLOR || button == MULTIPLAYER_LOBBY_BUTTON_BACK;

  return false;
}

static void multiplayer_state_select_next_lobby_button(int8_t step) {
  if (step == 0)
    return;

  uint8_t current = (uint8_t) selected_lobby_button;

  for (uint8_t tries = 0; tries < MULTIPLAYER_LOBBY_BUTTON_COUNT; tries++) {
    if (step > 0)
      current = (uint8_t) ((current + 1) % MULTIPLAYER_LOBBY_BUTTON_COUNT);
    else
      current = current == 0 ? (uint8_t) (MULTIPLAYER_LOBBY_BUTTON_COUNT - 1) : (uint8_t) (current - 1);

    if (multiplayer_state_lobby_button_is_selectable((multiplayer_lobby_button_t) current)) {
      selected_lobby_button = (multiplayer_lobby_button_t) current;
      return;
    }
  }
}

static void multiplayer_state_sync_selected_mode(void) {
  if (multiplayer_session_get_role() == MP_ROLE_CLIENT) {
    selected_lobby_button = MULTIPLAYER_LOBBY_BUTTON_PLAYER2_COLOR;
    return;
  }

  multiplayer_game_mode_t mode = multiplayer_session_get_mode();

  if (mode == MP_MODE_ARENA_BATTLE)
    selected_lobby_button = MULTIPLAYER_LOBBY_BUTTON_ARENA_BATTLE;
  else
    selected_lobby_button = MULTIPLAYER_LOBBY_BUTTON_SCORE_RACE;
}

static void multiplayer_state_activate_lobby_button(multiplayer_lobby_button_t button) {
  if (!multiplayer_state_lobby_button_is_selectable(button))
    return;

  selected_lobby_button = button;

  switch (button) {
    case MULTIPLAYER_LOBBY_BUTTON_SCORE_RACE:
      multiplayer_session_select_mode(MP_MODE_SCORE_RACE);
      break;

    case MULTIPLAYER_LOBBY_BUTTON_ARENA_BATTLE:
      multiplayer_session_select_mode(MP_MODE_ARENA_BATTLE);
      break;

    case MULTIPLAYER_LOBBY_BUTTON_PLAYER1_COLOR:
      multiplayer_session_select_next_player_skin(1);
      break;

    case MULTIPLAYER_LOBBY_BUTTON_PLAYER2_COLOR:
      multiplayer_session_select_next_player_skin(2);
      break;

    case MULTIPLAYER_LOBBY_BUTTON_START:
      multiplayer_session_start_game();
      break;

    case MULTIPLAYER_LOBBY_BUTTON_BACK:
      multiplayer_session_shutdown();
      state_set_current(STATE_MENU);
      break;

    default:
      break;
  }
}

static void multiplayer_state_adjust_selected_color(int8_t step) {
  if (selected_lobby_button == MULTIPLAYER_LOBBY_BUTTON_PLAYER1_COLOR) {
    if (step > 0)
      multiplayer_session_select_next_player_skin(1);
    else
      multiplayer_session_select_previous_player_skin(1);
  }
  else if (selected_lobby_button == MULTIPLAYER_LOBBY_BUTTON_PLAYER2_COLOR) {
    if (step > 0)
      multiplayer_session_select_next_player_skin(2);
    else
      multiplayer_session_select_previous_player_skin(2);
  }
}

multiplayer_lobby_button_t multiplayer_state_get_selected_lobby_button(void) {
  return selected_lobby_button;
}

void multiplayer_state_reset(multiplayer_role_t role) {
  multiplayer_session_reset(role);
  multiplayer_state_sync_selected_mode();
}

void multiplayer_state_handle_timer(void) {
  multiplayer_session_handle_timer();
}

void multiplayer_state_handle_keyboard(uint8_t scancode) {
  direction_t direction;

  if (scancode == STATE_KEY_ESC_BREAK) {
    multiplayer_session_shutdown();
    state_set_current(STATE_MENU);
    return;
  }

  if (multiplayer_session_get_phase() == MP_PHASE_LOBBY) {
    switch (scancode) {
      case STATE_KEY_1_MAKE:
        if (multiplayer_session_get_role() == MP_ROLE_HOST)
          multiplayer_state_activate_lobby_button(MULTIPLAYER_LOBBY_BUTTON_SCORE_RACE);
        return;

      case STATE_KEY_2_MAKE:
        if (multiplayer_session_get_role() == MP_ROLE_HOST)
          multiplayer_state_activate_lobby_button(MULTIPLAYER_LOBBY_BUTTON_ARENA_BATTLE);
        return;

      case STATE_KEY_W_MAKE:
      case STATE_KEY_UP_MAKE:
        multiplayer_state_select_next_lobby_button(-1);
        return;

      case STATE_KEY_S_MAKE:
      case STATE_KEY_DOWN_MAKE:
        multiplayer_state_select_next_lobby_button(1);
        return;

      case STATE_KEY_A_MAKE:
      case STATE_KEY_LEFT_MAKE:
        multiplayer_state_adjust_selected_color(-1);
        return;

      case STATE_KEY_D_MAKE:
      case STATE_KEY_RIGHT_MAKE:
        multiplayer_state_adjust_selected_color(1);
        return;

      case STATE_KEY_ENTER_MAKE:
        multiplayer_state_activate_lobby_button(selected_lobby_button);
        return;

      default:
        break;
    }
  }

  if (multiplayer_session_get_role() == MP_ROLE_HOST &&
      multiplayer_session_get_phase() == MP_PHASE_GAME_OVER) {
    if (scancode == STATE_KEY_ENTER_MAKE)
      multiplayer_session_return_to_lobby();
    return;
  }

  if (scancode == STATE_KEY_SPACE_MAKE) {
    multiplayer_session_handle_local_activate_powerup();
    return;
  }

  if (multiplayer_state_scancode_to_direction(scancode, &direction))
    multiplayer_session_handle_local_direction(direction);
}

void multiplayer_state_handle_mouse(const struct packet *packet) {
  (void) packet;

  uint16_t cursor_x = (uint16_t) mouse_cursor_get_x();
  uint16_t cursor_y = (uint16_t) mouse_cursor_get_y();

  if (multiplayer_session_get_phase() == MP_PHASE_LOBBY) {
    for (uint8_t i = 0; i < MULTIPLAYER_LOBBY_BUTTON_COUNT; i++) {
      multiplayer_lobby_button_t button_id = (multiplayer_lobby_button_t) i;
      ui_rect_t button = ui_layout_get_multiplayer_lobby_button(button_id);

      if (multiplayer_state_lobby_button_is_selectable(button_id) && ui_rect_contains(&button, cursor_x, cursor_y)) {
        selected_lobby_button = button_id;
        break;
      }
    }

    if (!mouse_cursor_was_left_clicked())
      return;

    for (uint8_t i = 0; i < MULTIPLAYER_LOBBY_BUTTON_COUNT; i++) {
      multiplayer_lobby_button_t button_id = (multiplayer_lobby_button_t) i;
      ui_rect_t button = ui_layout_get_multiplayer_lobby_button(button_id);

      if (!multiplayer_state_lobby_button_is_selectable(button_id) || !ui_rect_contains(&button, cursor_x, cursor_y))
        continue;

      selected_lobby_button = button_id;

      if (button_id == MULTIPLAYER_LOBBY_BUTTON_PLAYER1_COLOR || button_id == MULTIPLAYER_LOBBY_BUTTON_PLAYER2_COLOR) {
        uint16_t center = (uint16_t) (button.x + button.width / 2);

        if (cursor_x < center)
          multiplayer_state_adjust_selected_color(-1);
        else
          multiplayer_state_adjust_selected_color(1);

        return;
      }

      multiplayer_state_activate_lobby_button(button_id);
      return;
    }

    return;
  }

  if (!mouse_cursor_was_left_clicked())
    return;

  if (multiplayer_session_get_role() == MP_ROLE_HOST &&
      multiplayer_session_get_phase() == MP_PHASE_GAME_OVER) {
    ui_rect_t button = ui_layout_get_multiplayer_game_over_button();

    if (ui_rect_contains(&button, cursor_x, cursor_y))
      multiplayer_session_return_to_lobby();
    return;
  }

  if (multiplayer_session_get_phase() == MP_PHASE_PLAYING)
    multiplayer_session_handle_local_activate_powerup();
}

void multiplayer_state_handle_serial(void) {
  multiplayer_session_process_serial_rx();
}
