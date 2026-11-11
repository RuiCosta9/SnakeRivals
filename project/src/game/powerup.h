#ifndef POWERUP_H
#define POWERUP_H

#include <stdbool.h>
#include <stdint.h>

#include "food.h"
#include "snake.h"

/**
 * @file powerup.h
 * @brief Powerup item model and spawn/collision helpers.
 *
 * The module stores powerup entities on the grid. State modules decide how each
 * type affects gameplay when collected or activated.
 */

/**
 * @brief Powerup types supported by the project.
 */
typedef enum {
  POWERUP_DOUBLE_POINTS, /**< Temporarily increases score gained from food. */
  POWERUP_GHOST,         /**< Allows movement through self or enemy body rules that support it. */
  POWERUP_SHIELD,        /**< Arms one protection against a fatal collision. */
  POWERUP_MAGNET,        /**< Pulls food toward the snake head. */
  POWERUP_FRENZY,        /**< Enables extra food or accelerated scoring rules. */
  POWERUP_BONUS_FOOD,    /**< Grants bonus points immediately or while active. */
  POWERUP_SPEED_BOOST,   /**< Speeds up a multiplayer player. */
  POWERUP_SLOW_ENEMY,    /**< Slows an opponent in multiplayer. */
  POWERUP_FORCE_GROWTH,  /**< Forces enemy growth in multiplayer arena mode. */
  POWERUP_TYPE_COUNT     /**< Number of powerup types. */
} powerup_type_t;

/**
 * @brief Powerup item placed on a grid.
 */
typedef struct {
  position_t position;  /**< Grid position of the item. */
  powerup_type_t type;  /**< Type assigned when spawned. */
  uint16_t grid_width;  /**< Spawn grid width. */
  uint16_t grid_height; /**< Spawn grid height. */
  bool active;          /**< Whether the item is available to collect. */
} powerup_t;

/**
 * @brief Initialize an inactive powerup on a grid.
 *
 * @param powerup Powerup to initialize.
 * @param grid_width Grid width in cells.
 * @param grid_height Grid height in cells.
 */
void powerup_init(powerup_t *powerup, uint16_t grid_width, uint16_t grid_height);

/**
 * @brief Spawn a random powerup avoiding one snake and one food item.
 *
 * @param powerup Powerup to spawn.
 * @param snake Snake cells to avoid.
 * @param food Food cell to avoid.
 * @return true when a free cell was found.
 */
bool powerup_spawn(powerup_t *powerup, const snake_t *snake, const food_t *food);

/**
 * @brief Spawn a powerup from a restricted type list.
 *
 * @param powerup Powerup to spawn.
 * @param snake Primary snake cells to avoid.
 * @param other_snake Optional secondary snake cells to avoid.
 * @param food Optional food cell to avoid.
 * @param types Optional list of allowed types.
 * @param type_count Number of entries in @p types.
 * @return true when a free cell was found.
 */
bool powerup_spawn_from_types(powerup_t *powerup, const snake_t *snake, const snake_t *other_snake, const food_t *food, const powerup_type_t *types, uint8_t type_count);

/**
 * @brief Check whether an active powerup is on a snake head.
 *
 * @param powerup Powerup to inspect.
 * @param snake Snake to inspect.
 * @return true when the active item overlaps the head.
 */
bool powerup_collides_with_snake_head(const powerup_t *powerup, const snake_t *snake);

/**
 * @brief Mark a powerup as inactive.
 *
 * @param powerup Powerup to clear.
 */
void powerup_clear(powerup_t *powerup);

/**
 * @brief Get a powerup position.
 *
 * @param powerup Powerup to inspect.
 * @return Powerup position, or {0,0} for NULL.
 */
position_t powerup_get_position(const powerup_t *powerup);

/**
 * @brief Get a powerup type.
 *
 * @param powerup Powerup to inspect.
 * @return Powerup type, or POWERUP_DOUBLE_POINTS for NULL.
 */
powerup_type_t powerup_get_type(const powerup_t *powerup);

/**
 * @brief Check whether a powerup is active.
 *
 * @param powerup Powerup to inspect.
 * @return true if active.
 */
bool powerup_is_active(const powerup_t *powerup);

#endif
