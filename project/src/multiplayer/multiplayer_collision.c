/**
 * @file multiplayer_collision.c
 * @brief Multiplayer grid collision helper implementation.
 */

#include "multiplayer_collision.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arena_walls.h"

bool multiplayer_collision_positions_equal(position_t first, position_t second) {
  return first.x == second.x && first.y == second.y;
}

position_t multiplayer_collision_next_head_position(const snake_t *snake) {
  position_t head = snake_get_head(snake);

  if (snake == NULL)
    return head;

  switch (snake_get_next_direction(snake)) {
    case DIRECTION_UP:
      head.y--;
      break;

    case DIRECTION_RIGHT:
      head.x++;
      break;

    case DIRECTION_DOWN:
      head.y++;
      break;

    case DIRECTION_LEFT:
      head.x--;
      break;

    default:
      break;
  }

  return head;
}

bool multiplayer_collision_position_on_snake(position_t position, const snake_t *snake) {
  if (snake == NULL)
    return false;

  uint16_t length = snake_get_length(snake);

  for (uint16_t i = 0; i < length; i++) {
    if (multiplayer_collision_positions_equal(snake_get_segment(snake, i), position))
      return true;
  }

  return false;
}

bool multiplayer_collision_cell_is_solid(position_t position,
                                         multiplayer_game_mode_t mode,
                                         uint16_t grid_columns,
                                         uint16_t grid_rows) {
  if (position.x <= 0 || position.y <= 0)
    return true;

  if (position.x >= (int16_t) (grid_columns - 1) ||
      position.y >= (int16_t) (grid_rows - 1))
    return true;

  if (mode == MP_GAME_MODE_ARENA && arena_walls_cell_is_wall(position.x, position.y))
    return true;

  return false;
}

bool multiplayer_collision_next_head_hits_solid(const snake_t *snake,
                                                multiplayer_game_mode_t mode,
                                                uint16_t grid_columns,
                                                uint16_t grid_rows) {
  if (snake == NULL || !snake_is_alive(snake))
    return false;

  return multiplayer_collision_cell_is_solid(multiplayer_collision_next_head_position(snake),
                                             mode,
                                             grid_columns,
                                             grid_rows);
}
