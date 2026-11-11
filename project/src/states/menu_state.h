#ifndef MENU_STATE_H
#define MENU_STATE_H

#include <lcom/lcf.h>

#include <stdint.h>

#include "ui_layout.h"

/**
 * @file menu_state.h
 * @brief Main menu state input API.
 */

/**
 * @brief Get the currently selected main-menu button.
 *
 * @return Selected button.
 */
menu_button_t menu_state_get_selected(void);

/**
 * @brief Handle a timer tick for the main menu.
 */
void menu_state_handle_timer(void);

/**
 * @brief Handle main-menu keyboard navigation.
 *
 * @param scancode Keyboard scancode.
 */
void menu_state_handle_keyboard(uint8_t scancode);

/**
 * @brief Handle main-menu mouse hover/click input.
 *
 * @param packet Latest mouse packet.
 */
void menu_state_handle_mouse(const struct packet *packet);

#endif
