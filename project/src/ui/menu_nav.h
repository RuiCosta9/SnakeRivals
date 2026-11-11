#ifndef MENU_NAV_H
#define MENU_NAV_H

#include <stdbool.h>
#include <stdint.h>

#include "ui_layout.h"

/**
 * @file menu_nav.h
 * @brief Reusable keyboard and mouse navigation helpers for menus.
 */

/**
 * @brief Function that returns a rectangle for a button index.
 *
 * @param index Button index.
 * @return Button rectangle.
 */
typedef ui_rect_t (*menu_nav_rect_provider_t)(uint8_t index);

/**
 * @brief Update a selected index from keyboard input.
 *
 * @param scancode Keyboard scancode.
 * @param selected Selected index to mutate.
 * @param count Number of selectable entries.
 * @param activate Output flag set when the current entry should activate.
 * @return true when input affected selection or activation.
 */
bool menu_nav_handle_keyboard(uint8_t scancode, uint8_t *selected, uint8_t count, bool *activate);

/**
 * @brief Update menu hover selection from cursor position.
 *
 * @param provider Rectangle provider for each entry.
 * @param count Number of entries.
 * @param x Cursor x coordinate.
 * @param y Cursor y coordinate.
 * @param selected Selected index to mutate.
 */
void menu_nav_update_hover(menu_nav_rect_provider_t provider, uint8_t count, uint16_t x, uint16_t y, uint8_t *selected);

/**
 * @brief Handle mouse hover and click activation for a menu.
 *
 * @param provider Rectangle provider for each entry.
 * @param count Number of entries.
 * @param x Cursor x coordinate.
 * @param y Cursor y coordinate.
 * @param clicked Whether a click occurred.
 * @param selected Selected index to mutate.
 * @param activated Output activated index.
 * @return true when an entry was activated.
 */
bool menu_nav_handle_mouse(menu_nav_rect_provider_t provider, uint8_t count, uint16_t x, uint16_t y, bool clicked, uint8_t *selected, uint8_t *activated);

#endif
