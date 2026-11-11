#ifndef ASSETS_H
#define ASSETS_H

#include <stdbool.h>
#include <stdint.h>

#include "powerup.h"
#include "sprite.h"
#include "snake.h"

/**
 * @file assets.h
 * @brief Centralized sprite loading and lookup.
 *
 * Owns all XPM-backed sprites used by the renderer, including skin variants,
 * menu graphics, cursor, logo, food and powerup icons.
 */

/**
 * @brief Snake skin identifiers available to players.
 */
typedef enum {
  ASSETS_SNAKE_SKIN_GREEN,  /**< Green snake skin. */
  ASSETS_SNAKE_SKIN_RED,    /**< Red snake skin. */
  ASSETS_SNAKE_SKIN_BLUE,   /**< Blue snake skin. */
  ASSETS_SNAKE_SKIN_PURPLE, /**< Purple snake skin. */
  ASSETS_SNAKE_SKIN_YELLOW, /**< Yellow snake skin. */
  ASSETS_SNAKE_SKIN_COUNT   /**< Number of available skins. */
} assets_snake_skin_t;

/** @brief Load every sprite asset. @return 0 on success, non-zero on failure. */
int assets_load_all(void);

/** @brief Free every loaded sprite asset. */
void assets_free_all(void);

/** @brief Check whether assets are loaded. @return true if loaded. */
bool assets_are_loaded(void);

/** @brief Get the default snake head sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_snake_head(void);

/** @brief Get default snake head for a direction. @param direction Head direction. @return Sprite pointer or NULL. */
const sprite_t *assets_get_snake_head_for_direction(direction_t direction);

/** @brief Get snake head for a skin and direction. @param skin Skin index. @param direction Head direction. @return Sprite pointer or NULL. */
const sprite_t *assets_get_snake_head_for_skin_direction(uint8_t skin, direction_t direction);

/** @brief Get red snake head for a direction. @param direction Head direction. @return Sprite pointer or NULL. */
const sprite_t *assets_get_red_snake_head_for_direction(direction_t direction);

/** @brief Get the default snake body sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_snake_body(void);

/** @brief Get snake body for a skin. @param skin Skin index. @return Sprite pointer or NULL. */
const sprite_t *assets_get_snake_body_for_skin(uint8_t skin);

/** @brief Get red snake body sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_red_snake_body(void);

/** @brief Get the default snake tail sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_snake_tail(void);

/** @brief Get default snake tail for a direction. @param direction Tail direction. @return Sprite pointer or NULL. */
const sprite_t *assets_get_snake_tail_for_direction(direction_t direction);

/** @brief Get snake tail for a skin and direction. @param skin Skin index. @param direction Tail direction. @return Sprite pointer or NULL. */
const sprite_t *assets_get_snake_tail_for_skin_direction(uint8_t skin, direction_t direction);

/** @brief Get red snake tail for a direction. @param direction Tail direction. @return Sprite pointer or NULL. */
const sprite_t *assets_get_red_snake_tail_for_direction(direction_t direction);

/** @brief Get preview color for a snake skin. @param skin Skin index. @return RGB color. */
uint32_t assets_get_snake_skin_preview_color(uint8_t skin);

/** @brief Get display name for a snake skin. @param skin Skin index. @return Constant name. */
const char *assets_get_snake_skin_name(uint8_t skin);

/** @brief Get regular food sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_food(void);

/** @brief Get frenzy food sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_frenzy_food(void);

/** @brief Get generic powerup sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_power_up(void);

/** @brief Get powerup sprite for a type. @param type Powerup type. @return Sprite pointer or NULL. */
const sprite_t *assets_get_power_up_for_type(powerup_type_t type);

/** @brief Get tiled/background sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_background(void);

/** @brief Get menu background sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_menu(void);

/** @brief Get button sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_button(void);

/** @brief Get cursor sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_cursor(void);

/** @brief Get back-arrow sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_back_arrow(void);

/** @brief Get logo sprite. @return Sprite pointer or NULL. */
const sprite_t *assets_get_logo(void);

#endif
