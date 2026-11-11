#ifndef PLAY_MENU_STATE_H
#define PLAY_MENU_STATE_H

#include <lcom/lcf.h>

#include <stdint.h>

#include "ui_layout.h"

/**
 * @file play_menu_state.h
 * @brief Play-mode selection menu state API.
 */

/**
 * @brief Get the currently selected play-menu button.
 *
 * @return Selected button.
 */
play_menu_button_t play_menu_state_get_selected(void);

/**
 * @brief Handle a timer tick for the play menu.
 */
void play_menu_state_handle_timer(void);

/**
 * @brief Handle play-menu keyboard navigation.
 *
 * @param scancode Keyboard scancode.
 */
void play_menu_state_handle_keyboard(uint8_t scancode);

/**
 * @brief Handle play-menu mouse hover/click input.
 *
 * @param packet Latest mouse packet.
 */
void play_menu_state_handle_mouse(const struct packet *packet);

#endif
