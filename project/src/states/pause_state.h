#ifndef PAUSE_STATE_H
#define PAUSE_STATE_H

#include <lcom/lcf.h>

#include <stdint.h>

#include "ui_layout.h"

/**
 * @file pause_state.h
 * @brief Pause menu state API for singleplayer.
 */

/**
 * @brief Reset the pause menu selection to RESUME.
 *
 * @note Called automatically when entering STATE_PAUSE.
 */
void pause_state_reset(void);

/**
 * @brief Get the currently selected pause-menu button.
 *
 * @return Selected button.
 */
pause_button_t pause_state_get_selected(void);

/**
 * @brief Handle a timer tick for the pause menu.
 */
void pause_state_handle_timer(void);

/**
 * @brief Handle pause-menu keyboard navigation.
 *
 * @param scancode Keyboard scancode.
 */
void pause_state_handle_keyboard(uint8_t scancode);

/**
 * @brief Handle pause-menu mouse hover/click input.
 *
 * @param packet Latest mouse packet.
 */
void pause_state_handle_mouse(const struct packet *packet);

#endif
