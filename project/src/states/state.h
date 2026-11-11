#ifndef STATE_H
#define STATE_H

#include <lcom/lcf.h>

#include <stdint.h>

/**
 * @file state.h
 * @brief Global application state machine and event dispatch API.
 *
 * This module centralizes transitions between menus, gameplay modes and exit.
 * Hardware events are translated by app.c and then dispatched here.
 */

/**
 * @brief High-level screen or mode currently active in the application.
 */
typedef enum {
  STATE_MENU,              /**< Main menu. */
  STATE_PLAY_MENU,         /**< Menu used to choose local or multiplayer play. */
  STATE_INSTRUCTIONS,      /**< Instructions screen. */
  STATE_SINGLEPLAYER,      /**< Local Snake game. */
  STATE_MULTIPLAYER_JOIN,  /**< Room-code input screen for clients. */
  STATE_MULTIPLAYER_HOST,  /**< Multiplayer session controlled by the host. */
  STATE_MULTIPLAYER_CLIENT,/**< Multiplayer session controlled by a client. */
  STATE_PAUSE,             /**< Singleplayer pause menu. */
  STATE_GAME_OVER,         /**< Final result screen. */
  STATE_EXIT               /**< Sentinel state used to leave the main loop. */
} app_state_t;

#define STATE_KEY_ESC_BREAK   0x81
#define STATE_KEY_ENTER_MAKE  0x1C
#define STATE_KEY_BACKSPACE_MAKE 0x0E
#define STATE_KEY_SPACE_MAKE  0x39

#define STATE_KEY_1_MAKE      0x02
#define STATE_KEY_2_MAKE      0x03
#define STATE_KEY_3_MAKE      0x04
#define STATE_KEY_4_MAKE      0x05
#define STATE_KEY_5_MAKE      0x06
#define STATE_KEY_6_MAKE      0x07
#define STATE_KEY_7_MAKE      0x08
#define STATE_KEY_8_MAKE      0x09
#define STATE_KEY_9_MAKE      0x0A
#define STATE_KEY_0_MAKE      0x0B

#define STATE_KEY_C_MAKE      0x2E
#define STATE_KEY_H_MAKE      0x23
#define STATE_KEY_I_MAKE      0x17
#define STATE_KEY_J_MAKE      0x24
#define STATE_KEY_P_MAKE      0x19

#define STATE_KEY_W_MAKE      0x11
#define STATE_KEY_A_MAKE      0x1E
#define STATE_KEY_S_MAKE      0x1F
#define STATE_KEY_D_MAKE      0x20

#define STATE_KEY_UP_MAKE     0x48
#define STATE_KEY_LEFT_MAKE   0x4B
#define STATE_KEY_RIGHT_MAKE  0x4D
#define STATE_KEY_DOWN_MAKE   0x50

/**
 * @brief Reset the state machine to the main menu.
 *
 * @note Also clears any inactive multiplayer session.
 */
void state_init(void);

/**
 * @brief Get the active application state.
 *
 * @return Current state identifier.
 */
app_state_t state_get_current(void);

/**
 * @brief Change the active state.
 *
 * @param state New state to activate.
 *
 * @note Some transitions reset their target module, such as game over, pause,
 * singleplayer, join and multiplayer sessions.
 */
void state_set_current(app_state_t state);

/**
 * @brief Convert a state identifier into a stable debug string.
 *
 * @param state State identifier.
 * @return Constant string describing the state.
 */
const char *state_to_string(app_state_t state);

/**
 * @brief Dispatch one timer tick to the active state.
 */
void state_handle_timer_event(void);

/**
 * @brief Dispatch one keyboard scancode to the active state.
 *
 * @param scancode Keyboard scancode produced by the KBC driver.
 */
void state_handle_keyboard_event(uint8_t scancode);

/**
 * @brief Dispatch one completed mouse packet to the active state.
 *
 * @param packet Mouse packet produced by the mouse driver.
 */
void state_handle_mouse_event(const struct packet *packet);

/**
 * @brief Dispatch a serial interrupt event to the multiplayer state.
 *
 * @note Ignored unless the current state is a multiplayer state.
 */
void state_handle_serial_event(void);

#endif
