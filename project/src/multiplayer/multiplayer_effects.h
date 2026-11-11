#ifndef MULTIPLAYER_EFFECTS_H
#define MULTIPLAYER_EFFECTS_H

#include <stdbool.h>
#include <stdint.h>

#include "powerup.h"

/**
 * @file multiplayer_effects.h
 * @brief Timed multiplayer powerup effects.
 *
 * Owns host-side effect slots and durations. Snapshot generation copies these
 * effects so the client can render the same active state.
 */

/**
 * @brief Check whether a player has an active timed effect.
 *
 * @param player_index Zero-based player index.
 * @param type Effect type to search for.
 * @return true if the effect is active.
 */
bool multiplayer_effects_active(uint8_t player_index, powerup_type_t type);

/**
 * @brief Add or refresh a timed effect for a player.
 *
 * @param player_index Zero-based player index.
 * @param type Effect type.
 *
 * @note Mutates host effect slots and may evict the oldest effect if full.
 */
void multiplayer_effects_add(uint8_t player_index, powerup_type_t type);

/**
 * @brief Decrease all active effect timers by one tick.
 *
 * @note Mutates host effect slots and compacts inactive slots.
 */
void multiplayer_effects_tick_all(void);

/**
 * @brief Clear every host-side multiplayer effect slot.
 */
void multiplayer_effects_clear_all(void);

#endif
