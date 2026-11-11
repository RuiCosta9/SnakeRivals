#ifndef MAGNET_H
#define MAGNET_H

#include <stdbool.h>
#include <stdint.h>

#include "food.h"
#include "snake.h"

/**
 * @file magnet.h
 * @brief Reusable magnet movement helper for pulling food toward a target.
 *
 * The module is intentionally callback-driven so each game mode can define its
 * own blocked cells without coupling magnet logic to singleplayer or multiplayer.
 */

#define MAGNET_MAX_ANIMATION_POSITIONS 3

/**
 * @brief Callback used to test whether a magnet step is blocked.
 *
 * @param position Candidate position.
 * @param target Target position being approached.
 * @param context Caller-provided read-only context.
 * @return true if the candidate position cannot be used.
 */
typedef bool (*magnet_blocked_fn_t)(position_t position, position_t target, const void *context);

/**
 * @brief Callback invoked after food moves to a new position.
 *
 * @param position New food position.
 * @param context Caller-provided mutable context.
 */
typedef void (*magnet_moved_fn_t)(position_t position, void *context);

/**
 * @brief Pull an active food item toward a target by following a short path.
 *
 * @param food Food item to move.
 * @param target Target position, usually a snake head.
 * @param is_blocked Optional callback that rejects blocked cells.
 * @param blocked_context Context passed to @p is_blocked.
 * @param on_moved Optional callback invoked for every applied step.
 * @param moved_context Context passed to @p on_moved.
 * @return true if the food moved at least one cell.
 *
 * @note Mutates @p food when movement is possible.
 */
bool magnet_pull_food_towards(food_t *food, position_t target, magnet_blocked_fn_t is_blocked, const void *blocked_context, magnet_moved_fn_t on_moved, void *moved_context);

/**
 * @brief Build the short path the magnet would follow.
 *
 * @param out_positions Output array for path positions.
 * @param out_capacity Number of slots in @p out_positions.
 * @param start Initial position.
 * @param target Target position.
 * @param is_blocked Optional callback that rejects blocked cells.
 * @param blocked_context Context passed to @p is_blocked.
 * @return Number of positions written.
 */
uint8_t magnet_build_animation_path(position_t *out_positions, uint8_t out_capacity, position_t start, position_t target, magnet_blocked_fn_t is_blocked, const void *blocked_context);

#endif
