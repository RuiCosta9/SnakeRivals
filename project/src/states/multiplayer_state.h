#ifndef MULTIPLAYER_STATE_H
#define MULTIPLAYER_STATE_H

#include <lcom/lcf.h>

#include <stdint.h>

#include "multiplayer_session.h"
#include "ui_layout.h"

/**
 * @file multiplayer_state.h
 * @brief Event handlers for the multiplayer screens and match.
 *
 * Bridges UI input with the multiplayer session model. Lobby interaction,
 * gameplay input and serial events are forwarded to the session layer.
 */

/**
 * @brief Reset the multiplayer state and start a session with a role.
 *
 * @param role Role to use for the new session.
 *
 * @note Resets lobby selection according to the role.
 */
void multiplayer_state_reset(multiplayer_role_t role);

/**
 * @brief Advance multiplayer timers and session simulation.
 */
void multiplayer_state_handle_timer(void);

/**
 * @brief Handle keyboard input for lobby navigation and gameplay.
 *
 * @param scancode Keyboard scancode.
 */
void multiplayer_state_handle_keyboard(uint8_t scancode);

/**
 * @brief Handle mouse input for lobby buttons and powerup activation.
 *
 * @param packet Latest mouse packet.
 */
void multiplayer_state_handle_mouse(const struct packet *packet);

/**
 * @brief Process pending serial bytes in the multiplayer session.
 */
void multiplayer_state_handle_serial(void);

/**
 * @brief Get the currently highlighted lobby button.
 *
 * @return Selected lobby button identifier.
 */
multiplayer_lobby_button_t multiplayer_state_get_selected_lobby_button(void);

#endif
