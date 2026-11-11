#ifndef MOUSE_CURSOR_H
#define MOUSE_CURSOR_H

#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * @file mouse_cursor.h
 * @brief Logical mouse cursor used by menus and gameplay powerup activation.
 *
 * Converts mouse packets into a clamped screen position and click state. The
 * renderer reads the same state to draw the cursor.
 */

/**
 * @brief Center the cursor and clear button/movement state.
 */
void mouse_cursor_init(void);

/**
 * @brief Update cursor state from one complete mouse packet.
 *
 * @param packet Packet produced by the mouse driver.
 *
 * @note Mutates cursor position, movement flag, button state and click edge.
 */
void mouse_cursor_update(const struct packet *packet);

/**
 * @brief Get cursor x coordinate.
 *
 * @return Screen x coordinate.
 */
int16_t mouse_cursor_get_x(void);

/**
 * @brief Get cursor y coordinate.
 *
 * @return Screen y coordinate.
 */
int16_t mouse_cursor_get_y(void);

/**
 * @brief Check whether the left button was clicked on the last update.
 *
 * @return true only on the transition from up to down.
 */
bool mouse_cursor_was_left_clicked(void);

/**
 * @brief Check whether the left button is currently held.
 *
 * @return true if the button is down.
 */
bool mouse_cursor_is_left_down(void);

/**
 * @brief Check whether the cursor position changed on the last update.
 *
 * @return true if the cursor moved.
 */
bool mouse_cursor_has_moved(void);

#endif
