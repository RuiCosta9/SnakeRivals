/**
 * @file food.c
 * @brief Food spawning, collection checks and food-field implementation.
 */

#include "food.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static bool food_position_equals(position_t first, position_t second) {
  return first.x == second.x && first.y == second.y;
}

static bool food_position_is_on_snake(position_t position, const snake_t *snake) {
  if (snake == NULL)
    return false;

  uint16_t length = snake_get_length(snake);

  for (uint16_t i = 0; i < length; i++) {
    if (food_position_equals(position, snake_get_segment(snake, i)))
      return true;
  }

  return false;
}

void food_init(food_t *food, uint16_t grid_width, uint16_t grid_height) {
  if (food == NULL)
    return;

  food->position.x = 0;
  food->position.y = 0;
  food->grid_width = grid_width;
  food->grid_height = grid_height;
  food->active = false;
}

bool food_spawn(food_t *food, const snake_t *snake) {
  return food_spawn_avoiding(food, snake, NULL);
}

bool food_spawn_avoiding(food_t *food, const snake_t *snake, const snake_t *other_snake) {
  if (food == NULL)
    return false;

  if (food->grid_width == 0 || food->grid_height == 0)
    return false;

  uint32_t cell_count = (uint32_t) food->grid_width * food->grid_height;

  if (snake != NULL && snake_get_length(snake) >= cell_count) {
    food->active = false;
    return false;
  }

  uint32_t start = (uint32_t) rand() % cell_count;

  for (uint32_t offset = 0; offset < cell_count; offset++) {
    uint32_t cell = (start + offset) % cell_count;
    position_t position = {
      .x = (int16_t) (cell % food->grid_width),
      .y = (int16_t) (cell / food->grid_width)
    };

    if (!food_position_is_on_snake(position, snake) &&
        !food_position_is_on_snake(position, other_snake)) {
      food->position = position;
      food->active = true;
      return true;
    }
  }

  food->active = false;
  return false;
}

bool food_collides_with_snake_head(const food_t *food, const snake_t *snake) {
  if (food == NULL || snake == NULL)
    return false;

  if (!food->active)
    return false;

  return food_position_equals(food->position, snake_get_head(snake));
}

position_t food_get_position(const food_t *food) {
  position_t empty = {0, 0};

  if (food == NULL)
    return empty;

  return food->position;
}

bool food_is_active(const food_t *food) {
  if (food == NULL)
    return false;

  return food->active;
}

static bool food_field_cell_taken(const food_field_t *field, uint8_t skip_index, position_t position, const snake_t *snake, const food_t *avoid) {
  if (food_position_is_on_snake(position, snake))
    return true;

  if (avoid != NULL && avoid->active && food_position_equals(position, avoid->position))
    return true;

  for (uint8_t i = 0; i < field->count; i++) {
    if (i == skip_index)
      continue;

    if (field->items[i].active && food_position_equals(position, field->items[i].position))
      return true;
  }

  return false;
}

static bool food_field_place(food_field_t *field, uint8_t index, const snake_t *snake, const food_t *avoid) {
  if (field->grid_width == 0 || field->grid_height == 0)
    return false;

  uint32_t cell_count = (uint32_t) field->grid_width * field->grid_height;
  uint32_t start = (uint32_t) rand() % cell_count;

  for (uint32_t offset = 0; offset < cell_count; offset++) {
    uint32_t cell = (start + offset) % cell_count;
    position_t position = {
      .x = (int16_t) (cell % field->grid_width),
      .y = (int16_t) (cell / field->grid_width)
    };

    if (!food_field_cell_taken(field, index, position, snake, avoid)) {
      field->items[index].position = position;
      field->items[index].active = true;
      return true;
    }
  }

  field->items[index].active = false;
  return false;
}

void food_field_init(food_field_t *field, uint8_t count, uint16_t grid_width, uint16_t grid_height) {
  if (field == NULL)
    return;

  if (count > FOOD_FIELD_MAX)
    count = FOOD_FIELD_MAX;

  field->count = count;
  field->grid_width = grid_width;
  field->grid_height = grid_height;

  for (uint8_t i = 0; i < FOOD_FIELD_MAX; i++) {
    field->items[i].position.x = 0;
    field->items[i].position.y = 0;
    field->items[i].grid_width = grid_width;
    field->items[i].grid_height = grid_height;
    field->items[i].active = false;
  }
}

void food_field_clear(food_field_t *field) {
  if (field == NULL)
    return;

  for (uint8_t i = 0; i < FOOD_FIELD_MAX; i++)
    field->items[i].active = false;
}

bool food_field_spawn_all(food_field_t *field, const snake_t *snake, const food_t *avoid) {
  if (field == NULL)
    return false;

  bool all = true;

  for (uint8_t i = 0; i < field->count; i++) {
    if (!food_field_place(field, i, snake, avoid))
      all = false;
  }

  return all;
}

bool food_field_respawn(food_field_t *field, uint8_t index, const snake_t *snake, const food_t *avoid) {
  if (field == NULL || index >= field->count)
    return false;

  return food_field_place(field, index, snake, avoid);
}

int16_t food_field_collides_with_snake_head(const food_field_t *field, const snake_t *snake) {
  if (field == NULL || snake == NULL)
    return -1;

  position_t head = snake_get_head(snake);

  for (uint8_t i = 0; i < field->count; i++) {
    if (field->items[i].active && food_position_equals(field->items[i].position, head))
      return (int16_t) i;
  }

  return -1;
}

uint8_t food_field_get_count(const food_field_t *field) {
  if (field == NULL)
    return 0;

  return field->count;
}

bool food_field_item_active(const food_field_t *field, uint8_t index) {
  if (field == NULL || index >= field->count)
    return false;

  return field->items[index].active;
}

position_t food_field_item_position(const food_field_t *field, uint8_t index) {
  position_t empty = {0, 0};

  if (field == NULL || index >= field->count)
    return empty;

  return field->items[index].position;
}
