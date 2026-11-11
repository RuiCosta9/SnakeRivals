#ifndef INSTRUCTIONS_STATE_H
#define INSTRUCTIONS_STATE_H

#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * @file instructions_state.h
 * @brief Instructions screen state API.
 */

/**
 * @brief Check whether the back button is selected.
 *
 * @return true if selected.
 */
bool instructions_state_is_back_selected(void);

/**
 * @brief Handle a timer tick for the instructions screen.
 */
void instructions_state_handle_timer(void);

/**
 * @brief Handle instructions-screen keyboard input.
 *
 * @param scancode Keyboard scancode.
 */
void instructions_state_handle_keyboard(uint8_t scancode);

/**
 * @brief Handle instructions-screen mouse hover/click input.
 *
 * @param packet Latest mouse packet.
 */
void instructions_state_handle_mouse(const struct packet *packet);

#endif
