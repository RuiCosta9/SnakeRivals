#ifndef ARENA_WALLS_H
#define ARENA_WALLS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file arena_walls.h
 * @brief Static wall layout used by multiplayer Arena Battle mode.
 */

/**
 * @brief Get the number of wall blocks in the arena.
 *
 * @return Number of rectangular wall blocks.
 */
uint8_t arena_walls_get_count(void);

/**
 * @brief Get a rectangular arena wall block.
 *
 * @param index Wall block index.
 * @param x Output x coordinate in cells.
 * @param y Output y coordinate in cells.
 * @param w Output width in cells.
 * @param h Output height in cells.
 */
void arena_walls_get_block(uint8_t index, int16_t *x, int16_t *y, int16_t *w, int16_t *h);

/**
 * @brief Check whether a cell is part of an arena wall.
 *
 * @param cell_x Cell x coordinate.
 * @param cell_y Cell y coordinate.
 * @return true if the cell belongs to a static wall.
 */
bool arena_walls_cell_is_wall(int16_t cell_x, int16_t cell_y);

#endif
