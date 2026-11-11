/**
 * @file pause_state.c
 * @brief Pause menu navigation and singleplayer resume/restart handling.
 */

#include "pause_state.h"

#include "mouse_cursor.h"
#include "singleplayer_state.h"
#include "state.h"
#include "ui_layout.h"

/* Currently highlighted option. Starts on RESUME every time we pause. */
static pause_button_t selected = PAUSE_BUTTON_RESUME;

void pause_state_reset(void) {
  selected = PAUSE_BUTTON_RESUME;
}

pause_button_t pause_state_get_selected(void) {
  return selected;
}

/*
 * Runs the action of a given pause option.
 *
 *  - RESUME:    go back to the game WITHOUT resetting it (state.c keeps the
 *               game alive when going PAUSE -> SINGLEPLAYER), so the snake
 *               continues exactly where it stopped.
 *  - RESTART:   force a fresh game, then return to it.
 *  - MAIN_MENU: drop back to the main menu.
 *  - EXIT:      leave the application.
 */
static void pause_state_activate(pause_button_t option) {
  switch (option) {
    case PAUSE_BUTTON_RESUME:
      state_set_current(STATE_SINGLEPLAYER);
      break;

    case PAUSE_BUTTON_RESTART:
      singleplayer_state_reset();
      state_set_current(STATE_SINGLEPLAYER);
      break;

    case PAUSE_BUTTON_MAIN_MENU:
      state_set_current(STATE_MENU);
      break;

    case PAUSE_BUTTON_EXIT:
      state_set_current(STATE_EXIT);
      break;

    default:
      break;
  }
}

void pause_state_handle_timer(void) {
  /*
   * Nothing moves while paused. The game logic lives in
   * singleplayer_state_handle_timer, which is NOT called while we are in
   * STATE_PAUSE, so the snake naturally stays frozen.
   */
}

void pause_state_handle_keyboard(uint8_t scancode) {
  switch (scancode) {
    /* ESC (break) or P resume the game directly. */
    case STATE_KEY_ESC_BREAK:
    case STATE_KEY_P_MAKE:
      pause_state_activate(PAUSE_BUTTON_RESUME);
      break;

    /* Move the highlight up. */
    case STATE_KEY_W_MAKE:
    case STATE_KEY_UP_MAKE:
      selected = (pause_button_t) ((selected + PAUSE_BUTTON_COUNT - 1) % PAUSE_BUTTON_COUNT);
      break;

    /* Move the highlight down. */
    case STATE_KEY_S_MAKE:
    case STATE_KEY_DOWN_MAKE:
      selected = (pause_button_t) ((selected + 1) % PAUSE_BUTTON_COUNT);
      break;

    /* Confirm the highlighted option. */
    case STATE_KEY_ENTER_MAKE:
      pause_state_activate(selected);
      break;

    /* Number shortcuts: 1=Resume, 2=Restart, 3=Main Menu, 4=Exit. */
    case STATE_KEY_1_MAKE:
      pause_state_activate(PAUSE_BUTTON_RESUME);
      break;

    case STATE_KEY_2_MAKE:
      pause_state_activate(PAUSE_BUTTON_RESTART);
      break;

    case STATE_KEY_3_MAKE:
      pause_state_activate(PAUSE_BUTTON_MAIN_MENU);
      break;

    case STATE_KEY_4_MAKE:
      pause_state_activate(PAUSE_BUTTON_EXIT);
      break;

    default:
      break;
  }
}

void pause_state_handle_mouse(const struct packet *packet) {
  (void) packet;

  uint16_t cursor_x = (uint16_t) mouse_cursor_get_x();
  uint16_t cursor_y = (uint16_t) mouse_cursor_get_y();

  /* Hovering a button highlights it, so the keyboard and mouse stay in sync. */
  for (uint8_t i = 0; i < PAUSE_BUTTON_COUNT; i++) {
    ui_rect_t button = ui_layout_get_pause_button((pause_button_t) i);

    if (ui_rect_contains(&button, cursor_x, cursor_y)) {
      selected = (pause_button_t) i;
      break;
    }
  }

  if (!mouse_cursor_was_left_clicked())
    return;

  for (uint8_t i = 0; i < PAUSE_BUTTON_COUNT; i++) {
    ui_rect_t button = ui_layout_get_pause_button((pause_button_t) i);

    if (ui_rect_contains(&button, cursor_x, cursor_y)) {
      pause_state_activate((pause_button_t) i);
      return;
    }
  }
}
