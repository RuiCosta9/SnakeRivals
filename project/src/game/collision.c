/**
 * @file collision.c
 * @brief Singleplayer wall and self-collision implementation.
 */

#include "collision.h"

#include <stddef.h>

position_t collision_next_head_position(const snake_t *snake) {
  position_t head = snake_get_head(snake);

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

bool collision_is_out_of_bounds(position_t position, uint16_t grid_width, uint16_t grid_height) {
  return position.x < 0 || position.y < 0 ||
         position.x >= (int16_t) grid_width ||
         position.y >= (int16_t) grid_height;
}

bool collision_will_hit_wall(const snake_t *snake) {
  if (snake == NULL)
    return false;

  return collision_is_out_of_bounds(collision_next_head_position(snake), snake->grid_width, snake->grid_height);
}

bool collision_will_hit_self(const snake_t *snake) {
  if (snake == NULL)
    return false;

  uint16_t length = snake_get_length(snake);
  if (length <= 1)
    return false;

  position_t next_head = collision_next_head_position(snake);
  for (uint16_t i = 1; i + 1 < length; i++) {
    position_t segment = snake_get_segment(snake, i);
    if (segment.x == next_head.x && segment.y == next_head.y)
      return true;
  }

  return false;
}

bool collision_will_be_fatal(const snake_t *snake) {
  return collision_will_hit_wall(snake) || collision_will_hit_self(snake);
}
