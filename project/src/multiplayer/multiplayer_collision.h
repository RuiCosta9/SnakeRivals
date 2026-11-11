#ifndef MULTIPLAYER_COLLISION_H
#define MULTIPLAYER_COLLISION_H

#include <stdbool.h>
#include <stdint.h>

#include "multiplayer_session.h"
#include "snake.h"

/**
 * @file multiplayer_collision.h
 * @brief Collision helpers shared by multiplayer rule code.
 *
 * Extends basic snake collision checks with multiplayer-specific solid cells,
 * including arena borders and static Arena Battle walls.
 */

/**
 * @brief Compare two grid positions.
 *
 * @param first First position.
 * @param second Second position.
 * @return true if both coordinates match.
 */
bool multiplayer_collision_positions_equal(position_t first, position_t second);

/**
 * @brief Compute the next head position for a snake.
 *
 * @param snake Snake to inspect.
 * @return Next head position based on the queued direction.
 */
position_t multiplayer_collision_next_head_position(const snake_t *snake);

/**
 * @brief Check whether a position overlaps any segment of a snake.
 *
 * @param position Position to test.
 * @param snake Snake to inspect.
 * @return true when the position is on the snake.
 */
bool multiplayer_collision_position_on_snake(position_t position, const snake_t *snake);

/**
 * @brief Check whether a cell is solid for a multiplayer mode.
 *
 * @param position Cell to test.
 * @param mode Active multiplayer mode.
 * @param grid_columns Grid width.
 * @param grid_rows Grid height.
 * @return true if the cell is blocked by border or arena walls.
 */
bool multiplayer_collision_cell_is_solid(position_t position, multiplayer_game_mode_t mode, uint16_t grid_columns, uint16_t grid_rows);

/**
 * @brief Check whether the next snake head would hit a solid cell.
 *
 * @param snake Snake to inspect.
 * @param mode Active multiplayer mode.
 * @param grid_columns Grid width.
 * @param grid_rows Grid height.
 * @return true if the next move hits a solid cell.
 */
bool multiplayer_collision_next_head_hits_solid(const snake_t *snake, multiplayer_game_mode_t mode, uint16_t grid_columns, uint16_t grid_rows);

#endif
