#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include <stdint.h>

#include "snake.h"

/**
 * @file collision.h
 * @brief Singleplayer collision helpers.
 *
 * Computes the next snake head position and checks wall/self collisions using
 * the grid dimensions stored in the snake model.
 */

/**
 * @brief Compute where the snake head would move next.
 *
 * @param snake Snake to inspect.
 * @return Next head position according to the queued direction.
 */
position_t collision_next_head_position(const snake_t *snake);

/**
 * @brief Check whether a position is outside a grid.
 *
 * @param position Position to inspect.
 * @param grid_width Grid width in cells.
 * @param grid_height Grid height in cells.
 * @return true if outside the grid.
 */
bool collision_is_out_of_bounds(position_t position, uint16_t grid_width, uint16_t grid_height);

/**
 * @brief Check whether the snake's next move hits a wall.
 *
 * @param snake Snake to inspect.
 * @return true if the next head position is outside the snake grid.
 */
bool collision_will_hit_wall(const snake_t *snake);

/**
 * @brief Check whether the snake's next move hits its own body.
 *
 * @param snake Snake to inspect.
 * @return true if the next head position overlaps a body segment.
 */
bool collision_will_hit_self(const snake_t *snake);

/**
 * @brief Check whether the next move is fatal in normal singleplayer rules.
 *
 * @param snake Snake to inspect.
 * @return true if the next move hits wall or self.
 */
bool collision_will_be_fatal(const snake_t *snake);

#endif
