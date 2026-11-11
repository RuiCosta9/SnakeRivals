#ifndef GAME_OVER_STATE_H
#define GAME_OVER_STATE_H

#include <lcom/lcf.h>

#include <stdint.h>

#include "ui_layout.h"

/**
 * @file game_over_state.h
 * @brief Game-over screen state API.
 */

/**
 * @brief Reset the game-over menu selection.
 */
void game_over_state_reset(void);

/**
 * @brief Get the currently selected game-over button.
 *
 * @return Selected button.
 */
game_over_button_t game_over_state_get_selected(void);

/**
 * @brief Handle a timer tick for the game-over screen.
 */
void game_over_state_handle_timer(void);

/**
 * @brief Handle game-over keyboard navigation.
 *
 * @param scancode Keyboard scancode.
 */
void game_over_state_handle_keyboard(uint8_t scancode);

/**
 * @brief Handle game-over mouse hover/click input.
 *
 * @param packet Latest mouse packet.
 */
void game_over_state_handle_mouse(const struct packet *packet);

#endif
