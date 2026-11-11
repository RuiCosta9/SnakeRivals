#ifndef JOIN_STATE_H
#define JOIN_STATE_H

#include <lcom/lcf.h>

#include <stdint.h>

#include "ui_layout.h"

/**
 * @file join_state.h
 * @brief Room-code input state for joining multiplayer sessions.
 */

/**
 * @brief Reset typed code, status text and selected button.
 */
void join_state_reset(void);

/**
 * @brief Get the currently selected join-screen button.
 *
 * @return Selected button.
 */
join_button_t join_state_get_selected(void);

/**
 * @brief Handle a timer tick for the join screen.
 */
void join_state_handle_timer(void);

/**
 * @brief Handle code input and join-screen keyboard navigation.
 *
 * @param scancode Keyboard scancode.
 */
void join_state_handle_keyboard(uint8_t scancode);

/**
 * @brief Handle join-screen mouse hover/click input.
 *
 * @param packet Latest mouse packet.
 */
void join_state_handle_mouse(const struct packet *packet);

/**
 * @brief Get the room-code text currently displayed.
 *
 * @return Null-terminated code text.
 */
const char *join_state_get_code_text(void);

/**
 * @brief Get the current join status message.
 *
 * @return Null-terminated status text.
 */
const char *join_state_get_status_text(void);

#endif
