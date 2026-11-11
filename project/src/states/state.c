/**
 * @file state.c
 * @brief Global state machine and event dispatcher implementation.
 */

#include "state.h"

#include "game_over_state.h"
#include "instructions_state.h"
#include "join_state.h"
#include "menu_state.h"
#include "multiplayer_state.h"
#include "pause_state.h"
#include "play_menu_state.h"
#include "singleplayer_state.h"

static app_state_t current_state = STATE_MENU;

void state_init(void) {
  current_state = STATE_MENU;
  multiplayer_state_reset(MP_ROLE_NONE);
}

app_state_t state_get_current(void) {
  return current_state;
}

void state_set_current(app_state_t state) {
  if (state == STATE_GAME_OVER && current_state != STATE_GAME_OVER)
    game_over_state_reset();

  if (state == STATE_PAUSE && current_state != STATE_PAUSE)
    pause_state_reset();

  if (state == STATE_SINGLEPLAYER && current_state != STATE_SINGLEPLAYER && current_state != STATE_PAUSE)
    singleplayer_state_reset();
  else if (state == STATE_MULTIPLAYER_JOIN && current_state != STATE_MULTIPLAYER_JOIN)
    join_state_reset();
  else if (state == STATE_MULTIPLAYER_HOST && current_state != STATE_MULTIPLAYER_HOST)
    multiplayer_state_reset(MP_ROLE_HOST);
  else if (state == STATE_MULTIPLAYER_CLIENT && current_state != STATE_MULTIPLAYER_CLIENT)
    multiplayer_state_reset(MP_ROLE_CLIENT);
  else if ((current_state == STATE_MULTIPLAYER_HOST || current_state == STATE_MULTIPLAYER_CLIENT) &&
           state != STATE_MULTIPLAYER_HOST && state != STATE_MULTIPLAYER_CLIENT)
    multiplayer_state_reset(MP_ROLE_NONE);
  current_state = state;
}

const char *state_to_string(app_state_t state) {
  switch (state) {
    case STATE_MENU:
      return "STATE_MENU";

    case STATE_PLAY_MENU:
      return "STATE_PLAY_MENU";

    case STATE_INSTRUCTIONS:
      return "STATE_INSTRUCTIONS";

    case STATE_SINGLEPLAYER:
      return "STATE_SINGLEPLAYER";

    case STATE_MULTIPLAYER_JOIN:
      return "STATE_MULTIPLAYER_JOIN";

    case STATE_MULTIPLAYER_HOST:
      return "STATE_MULTIPLAYER_HOST";

    case STATE_MULTIPLAYER_CLIENT:
      return "STATE_MULTIPLAYER_CLIENT";

    case STATE_PAUSE:
      return "STATE_PAUSE";

    case STATE_GAME_OVER:
      return "STATE_GAME_OVER";

    case STATE_EXIT:
      return "STATE_EXIT";

    default:
      return "STATE_UNKNOWN";
  }
}

void state_handle_timer_event(void) {
  switch (current_state) {
    case STATE_MENU:
      menu_state_handle_timer();
      break;

    case STATE_PLAY_MENU:
      play_menu_state_handle_timer();
      break;

    case STATE_INSTRUCTIONS:
      instructions_state_handle_timer();
      break;

    case STATE_SINGLEPLAYER:
      singleplayer_state_handle_timer();
      break;

    case STATE_MULTIPLAYER_JOIN:
      join_state_handle_timer();
      break;

    case STATE_MULTIPLAYER_HOST:
    case STATE_MULTIPLAYER_CLIENT:
      multiplayer_state_handle_timer();
      break;

    case STATE_PAUSE:
      pause_state_handle_timer();
      break;

    case STATE_GAME_OVER:
      game_over_state_handle_timer();
      break;

    case STATE_EXIT:
    default:
      break;
  }
}

void state_handle_keyboard_event(uint8_t scancode) {
  switch (current_state) {
    case STATE_MENU:
      menu_state_handle_keyboard(scancode);
      break;

    case STATE_PLAY_MENU:
      play_menu_state_handle_keyboard(scancode);
      break;

    case STATE_INSTRUCTIONS:
      instructions_state_handle_keyboard(scancode);
      break;

    case STATE_SINGLEPLAYER:
      singleplayer_state_handle_keyboard(scancode);
      break;

    case STATE_MULTIPLAYER_JOIN:
      join_state_handle_keyboard(scancode);
      break;

    case STATE_MULTIPLAYER_HOST:
    case STATE_MULTIPLAYER_CLIENT:
      multiplayer_state_handle_keyboard(scancode);
      break;

    case STATE_PAUSE:
      pause_state_handle_keyboard(scancode);
      break;

    case STATE_GAME_OVER:
      game_over_state_handle_keyboard(scancode);
      break;

    case STATE_EXIT:
    default:
      break;
  }
}

void state_handle_mouse_event(const struct packet *packet) {
  switch (current_state) {
    case STATE_MENU:
      menu_state_handle_mouse(packet);
      break;

    case STATE_PLAY_MENU:
      play_menu_state_handle_mouse(packet);
      break;

    case STATE_INSTRUCTIONS:
      instructions_state_handle_mouse(packet);
      break;

    case STATE_SINGLEPLAYER:
      singleplayer_state_handle_mouse(packet);
      break;

    case STATE_MULTIPLAYER_JOIN:
      join_state_handle_mouse(packet);
      break;

    case STATE_MULTIPLAYER_HOST:
    case STATE_MULTIPLAYER_CLIENT:
      multiplayer_state_handle_mouse(packet);
      break;

    case STATE_PAUSE:
      pause_state_handle_mouse(packet);
      break;

    case STATE_GAME_OVER:
      game_over_state_handle_mouse(packet);
      break;

    case STATE_EXIT:
    default:
      break;
  }
}

void state_handle_serial_event(void) {
  switch (current_state) {
    case STATE_MULTIPLAYER_HOST:
    case STATE_MULTIPLAYER_CLIENT:
      multiplayer_state_handle_serial();
      break;

    case STATE_MENU:
    case STATE_PLAY_MENU:
    case STATE_INSTRUCTIONS:
    case STATE_SINGLEPLAYER:
    case STATE_MULTIPLAYER_JOIN:
    case STATE_PAUSE:
    case STATE_GAME_OVER:
    case STATE_EXIT:
    default:
      break;
  }
}
