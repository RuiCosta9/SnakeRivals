/**
 * @file powerup.c
 * @brief Powerup spawning, collision and query implementation.
 */

#include "powerup.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static bool powerup_position_equals(position_t first, position_t second) {
  return first.x == second.x && first.y == second.y;
}

static bool powerup_position_is_on_snake(position_t position, const snake_t *snake) {
  if (snake == NULL)
    return false;

  uint16_t length = snake_get_length(snake);

  for (uint16_t i = 0; i < length; i++) {
    if (powerup_position_equals(position, snake_get_segment(snake, i)))
      return true;
  }

  return false;
}

static bool powerup_position_is_on_food(position_t position, const food_t *food) {
  if (food == NULL || !food_is_active(food))
    return false;

  return powerup_position_equals(position, food_get_position(food));
}

static powerup_type_t powerup_pick_type(const powerup_type_t *types, uint8_t type_count) {
  if (types == NULL || type_count == 0)
    return (powerup_type_t) (rand() % POWERUP_TYPE_COUNT);

  return types[rand() % type_count];
}

void powerup_init(powerup_t *powerup, uint16_t grid_width, uint16_t grid_height) {
  if (powerup == NULL)
    return;

  powerup->position.x = 0;
  powerup->position.y = 0;
  powerup->type = POWERUP_DOUBLE_POINTS;
  powerup->grid_width = grid_width;
  powerup->grid_height = grid_height;
  powerup->active = false;
}

bool powerup_spawn(powerup_t *powerup, const snake_t *snake, const food_t *food) {
  return powerup_spawn_from_types(powerup, snake, NULL, food, NULL, 0);
}

bool powerup_spawn_from_types(powerup_t *powerup, const snake_t *snake, const snake_t *other_snake, const food_t *food, const powerup_type_t *types, uint8_t type_count) {
  if (powerup == NULL)
    return false;

  if (powerup->grid_width == 0 || powerup->grid_height == 0)
    return false;

  uint32_t cell_count = (uint32_t) powerup->grid_width * powerup->grid_height;

  /* Pick a random starting cell, then scan forward until a free one is found. */
  uint32_t start = (uint32_t) rand() % cell_count;

  for (uint32_t offset = 0; offset < cell_count; offset++) {
    uint32_t cell = (start + offset) % cell_count;
    position_t position = {
      .x = (int16_t) (cell % powerup->grid_width),
      .y = (int16_t) (cell / powerup->grid_width)
    };

    if (!powerup_position_is_on_snake(position, snake) &&
        !powerup_position_is_on_snake(position, other_snake) &&
        !powerup_position_is_on_food(position, food)) {
      powerup->position = position;
      powerup->type = powerup_pick_type(types, type_count);
      powerup->active = true;
      return true;
    }
  }

  powerup->active = false;
  return false;
}

bool powerup_collides_with_snake_head(const powerup_t *powerup, const snake_t *snake) {
  if (powerup == NULL || snake == NULL)
    return false;

  if (!powerup->active)
    return false;

  return powerup_position_equals(powerup->position, snake_get_head(snake));
}

void powerup_clear(powerup_t *powerup) {
  if (powerup == NULL)
    return;

  powerup->active = false;
}

position_t powerup_get_position(const powerup_t *powerup) {
  position_t empty = {0, 0};

  if (powerup == NULL)
    return empty;

  return powerup->position;
}

powerup_type_t powerup_get_type(const powerup_t *powerup) {
  if (powerup == NULL)
    return POWERUP_DOUBLE_POINTS;

  return powerup->type;
}

bool powerup_is_active(const powerup_t *powerup) {
  if (powerup == NULL)
    return false;

  return powerup->active;
}
