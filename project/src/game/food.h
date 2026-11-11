#ifndef FOOD_H
#define FOOD_H

#include <stdbool.h>
#include <stdint.h>

#include "snake.h"

/**
 * @file food.h
 * @brief Food entities and food-field helpers.
 *
 * Provides grid placement and head-collision checks for regular food and small
 * collections of extra food used by frenzy-style effects.
 */

#define FOOD_FIELD_MAX 8

/**
 * @brief Single food item placed on a grid.
 */
typedef struct {
  position_t position;  /**< Grid position of the item. */
  uint16_t grid_width;  /**< Spawn grid width. */
  uint16_t grid_height; /**< Spawn grid height. */
  bool active;          /**< Whether the food can be collected. */
} food_t;

/**
 * @brief Fixed-size collection of food items.
 */
typedef struct {
  food_t items[FOOD_FIELD_MAX]; /**< Food slots. */
  uint8_t count;                /**< Number of configured slots. */
  uint16_t grid_width;          /**< Spawn grid width. */
  uint16_t grid_height;         /**< Spawn grid height. */
} food_field_t;

/**
 * @brief Initialize an inactive food item.
 *
 * @param food Food item to initialize.
 * @param grid_width Grid width in cells.
 * @param grid_height Grid height in cells.
 */
void food_init(food_t *food, uint16_t grid_width, uint16_t grid_height);

/**
 * @brief Spawn food in a free cell outside one snake.
 *
 * @param food Food item to spawn.
 * @param snake Snake cells to avoid.
 * @return true when a free cell was found.
 */
bool food_spawn(food_t *food, const snake_t *snake);

/**
 * @brief Spawn food avoiding two snakes.
 *
 * @param food Food item to spawn.
 * @param snake Primary snake cells to avoid.
 * @param other_snake Optional secondary snake cells to avoid.
 * @return true when a free cell was found.
 */
bool food_spawn_avoiding(food_t *food, const snake_t *snake, const snake_t *other_snake);

/**
 * @brief Check whether active food overlaps a snake head.
 *
 * @param food Food item to inspect.
 * @param snake Snake to inspect.
 * @return true when active food is at the snake head.
 */
bool food_collides_with_snake_head(const food_t *food, const snake_t *snake);

/**
 * @brief Get the food position.
 *
 * @param food Food item to inspect.
 * @return Food position, or {0,0} for NULL.
 */
position_t food_get_position(const food_t *food);

/**
 * @brief Check whether a food item is active.
 *
 * @param food Food item to inspect.
 * @return true if active.
 */
bool food_is_active(const food_t *food);

/**
 * @brief Initialize a food field with a requested number of slots.
 *
 * @param field Field to initialize.
 * @param count Number of active slots, clamped by FOOD_FIELD_MAX.
 * @param grid_width Grid width in cells.
 * @param grid_height Grid height in cells.
 */
void food_field_init(food_field_t *field, uint8_t count, uint16_t grid_width, uint16_t grid_height);

/**
 * @brief Clear all items in a food field.
 *
 * @param field Field to clear.
 */
void food_field_clear(food_field_t *field);

/**
 * @brief Spawn every configured food-field item.
 *
 * @param field Field to populate.
 * @param snake Snake cells to avoid.
 * @param avoid Optional single food item to avoid.
 * @return true when every item was spawned.
 */
bool food_field_spawn_all(food_field_t *field, const snake_t *snake, const food_t *avoid);

/**
 * @brief Respawn one food-field item.
 *
 * @param field Field to update.
 * @param index Item index.
 * @param snake Snake cells to avoid.
 * @param avoid Optional single food item to avoid.
 * @return true when the item was respawned.
 */
bool food_field_respawn(food_field_t *field, uint8_t index, const snake_t *snake, const food_t *avoid);

/**
 * @brief Find which food-field item overlaps a snake head.
 *
 * @param field Field to inspect.
 * @param snake Snake to inspect.
 * @return Item index, or -1 if no active item collides.
 */
int16_t food_field_collides_with_snake_head(const food_field_t *field, const snake_t *snake);

/**
 * @brief Get the number of configured food-field slots.
 *
 * @param field Field to inspect.
 * @return Number of slots, or 0 for NULL.
 */
uint8_t food_field_get_count(const food_field_t *field);

/**
 * @brief Check whether a food-field item is active.
 *
 * @param field Field to inspect.
 * @param index Item index.
 * @return true if the indexed item is active.
 */
bool food_field_item_active(const food_field_t *field, uint8_t index);

/**
 * @brief Get a food-field item position.
 *
 * @param field Field to inspect.
 * @param index Item index.
 * @return Item position, or {0,0} for invalid input.
 */
position_t food_field_item_position(const food_field_t *field, uint8_t index);

#endif
