/**
 * @file magnet.c
 * @brief Callback-based food magnet movement implementation.
 */

#include "magnet.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static bool magnet_position_equals(position_t first, position_t second) {
  return first.x == second.x && first.y == second.y;
}

static bool magnet_step_position(position_t *position,
                                 position_t target,
                                 bool horizontal,
                                 magnet_blocked_fn_t is_blocked,
                                 const void *blocked_context) {
  if (position == NULL)
    return false;

  position_t next = *position;
  int16_t dx = target.x - position->x;
  int16_t dy = target.y - position->y;

  if (horizontal) {
    if (dx == 0)
      return false;

    next.x += dx > 0 ? 1 : -1;
  }
  else {
    if (dy == 0)
      return false;

    next.y += dy > 0 ? 1 : -1;
  }

  if (is_blocked != NULL && is_blocked(next, target, blocked_context))
    return false;

  *position = next;
  return true;
}

uint8_t magnet_build_animation_path(position_t *out_positions,
                                    uint8_t out_capacity,
                                    position_t start,
                                    position_t target,
                                    magnet_blocked_fn_t is_blocked,
                                    const void *blocked_context) {
  position_t current = start;
  uint8_t count = 0;

  if (out_positions != NULL && out_capacity > 0) {
    out_positions[0] = current;
    count = 1;
  }

  for (uint8_t step = 0; step < 2; step++) {
    int16_t dx = target.x - current.x;
    int16_t dy = target.y - current.y;

    if (dx == 0 && dy == 0)
      break;

    int16_t adx = dx < 0 ? -dx : dx;
    int16_t ady = dy < 0 ? -dy : dy;
    bool horizontal_first = adx >= ady;
    position_t before = current;

    if (!magnet_step_position(&current, target, horizontal_first, is_blocked, blocked_context))
      (void) magnet_step_position(&current, target, !horizontal_first, is_blocked, blocked_context);

    if (magnet_position_equals(before, current))
      continue;

    if (out_positions != NULL && count < out_capacity) {
      out_positions[count] = current;
      count++;
    }
    else if (out_positions != NULL && out_capacity > 0)
      out_positions[out_capacity - 1] = current;
  }

  return count;
}

bool magnet_pull_food_towards(food_t *food,
                              position_t target,
                              magnet_blocked_fn_t is_blocked,
                              const void *blocked_context,
                              magnet_moved_fn_t on_moved,
                              void *moved_context) {
  if (food == NULL || !food_is_active(food))
    return false;

  position_t path[MAGNET_MAX_ANIMATION_POSITIONS];
  uint8_t count = magnet_build_animation_path(path,
                                              (uint8_t) (sizeof(path) / sizeof(path[0])),
                                              food->position,
                                              target,
                                              is_blocked,
                                              blocked_context);

  if (count <= 1)
    return false;

  for (uint8_t i = 1; i < count; i++) {
    food->position = path[i];

    if (on_moved != NULL)
      on_moved(path[i], moved_context);
  }

  return true;
}
