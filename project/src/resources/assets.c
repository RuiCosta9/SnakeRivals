/**
 * @file assets.c
 * @brief Sprite asset loading, recoloring and lookup implementation.
 */

#include "assets.h"

#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>

#include "sprite.h"

#include "xpms/snake.xpm"
#include "xpms/powerups.xpm"
#include "xpms/ui.xpm"
#include "xpms/snake_rivals_logo.xpm"

#define SNAKE_DIR_COUNT 4

static sprite_t *snake_head = NULL;
static sprite_t *snake_head_up = NULL;
static sprite_t *snake_head_down = NULL;
static sprite_t *snake_head_left = NULL;
static sprite_t *snake_body = NULL;
static sprite_t *snake_tail = NULL;
static sprite_t *snake_tail_up = NULL;
static sprite_t *snake_tail_down = NULL;
static sprite_t *snake_tail_left = NULL;

static sprite_t *snake_skin_heads[ASSETS_SNAKE_SKIN_COUNT][SNAKE_DIR_COUNT];
static sprite_t *snake_skin_bodies[ASSETS_SNAKE_SKIN_COUNT];
static sprite_t *snake_skin_tails[ASSETS_SNAKE_SKIN_COUNT][SNAKE_DIR_COUNT];

static sprite_t *food = NULL;
static sprite_t *frenzy_food = NULL;
static sprite_t *power_up = NULL;
static sprite_t *power_up_sprites[POWERUP_TYPE_COUNT];
static sprite_t *background = NULL;
static sprite_t *menu = NULL;
static sprite_t *button = NULL;
static sprite_t *cursor = NULL;
static sprite_t *back_arrow = NULL;
static sprite_t *logo = NULL;

static bool loaded = false;

static uint8_t assets_clamp_skin(uint8_t skin) {
  return skin < ASSETS_SNAKE_SKIN_COUNT ? skin : ASSETS_SNAKE_SKIN_GREEN;
}

static uint8_t assets_direction_index(direction_t direction) {
  switch (direction) {
    case DIRECTION_UP:
      return 0;
    case DIRECTION_RIGHT:
      return 1;
    case DIRECTION_DOWN:
      return 2;
    case DIRECTION_LEFT:
      return 3;
    default:
      return 1;
  }
}

static void assets_identity_color_map(uint8_t color_map[256]) {
  for (uint16_t i = 0; i < 256; i++)
    color_map[i] = (uint8_t) i;
}

static void assets_prepare_snake_color_map(uint8_t skin, uint8_t color_map[256]) {
  assets_identity_color_map(color_map);

  switch (skin) {
    case ASSETS_SNAKE_SKIN_RED:
      color_map[2] = 4;
      color_map[6] = 4;
      color_map[10] = 4;
      color_map[11] = 5;
      color_map[14] = 12;
      break;

    case ASSETS_SNAKE_SKIN_BLUE:
      color_map[2] = 1;
      color_map[6] = 1;
      color_map[10] = 9;
      color_map[11] = 11;
      color_map[14] = 15;
      break;

    case ASSETS_SNAKE_SKIN_PURPLE:
      color_map[2] = 5;
      color_map[6] = 5;
      color_map[10] = 13;
      color_map[11] = 5;
      color_map[14] = 15;
      break;

    case ASSETS_SNAKE_SKIN_YELLOW:
      color_map[2] = 6;
      color_map[6] = 6;
      color_map[10] = 14;
      color_map[11] = 14;
      color_map[14] = 15;
      break;

    case ASSETS_SNAKE_SKIN_GREEN:
    default:
      break;
  }
}

static int assets_load_recolored_skin(uint8_t skin) {
  uint8_t color_map[256];

  assets_prepare_snake_color_map(skin, color_map);

  snake_skin_heads[skin][assets_direction_index(DIRECTION_RIGHT)] = sprite_create_recolored((xpm_map_t) snake_head_right_xpm, XPM_INDEXED, 0, true, color_map);
  snake_skin_heads[skin][assets_direction_index(DIRECTION_UP)] = sprite_create_recolored((xpm_map_t) snake_head_up_xpm, XPM_INDEXED, 0, true, color_map);
  snake_skin_heads[skin][assets_direction_index(DIRECTION_DOWN)] = sprite_create_recolored((xpm_map_t) snake_head_down_xpm, XPM_INDEXED, 0, true, color_map);
  snake_skin_heads[skin][assets_direction_index(DIRECTION_LEFT)] = sprite_create_recolored((xpm_map_t) snake_head_left_xpm, XPM_INDEXED, 0, true, color_map);
  snake_skin_bodies[skin] = sprite_create_recolored((xpm_map_t) snake_body_xpm, XPM_INDEXED, 0, true, color_map);
  snake_skin_tails[skin][assets_direction_index(DIRECTION_RIGHT)] = sprite_create_recolored((xpm_map_t) snake_tail_right_xpm, XPM_INDEXED, 0, true, color_map);
  snake_skin_tails[skin][assets_direction_index(DIRECTION_UP)] = sprite_create_recolored((xpm_map_t) snake_tail_up_xpm, XPM_INDEXED, 0, true, color_map);
  snake_skin_tails[skin][assets_direction_index(DIRECTION_DOWN)] = sprite_create_recolored((xpm_map_t) snake_tail_down_xpm, XPM_INDEXED, 0, true, color_map);
  snake_skin_tails[skin][assets_direction_index(DIRECTION_LEFT)] = sprite_create_recolored((xpm_map_t) snake_tail_left_xpm, XPM_INDEXED, 0, true, color_map);

  return snake_skin_heads[skin][assets_direction_index(DIRECTION_RIGHT)] == NULL ||
         snake_skin_heads[skin][assets_direction_index(DIRECTION_UP)] == NULL ||
         snake_skin_heads[skin][assets_direction_index(DIRECTION_DOWN)] == NULL ||
         snake_skin_heads[skin][assets_direction_index(DIRECTION_LEFT)] == NULL ||
         snake_skin_bodies[skin] == NULL ||
         snake_skin_tails[skin][assets_direction_index(DIRECTION_RIGHT)] == NULL ||
         snake_skin_tails[skin][assets_direction_index(DIRECTION_UP)] == NULL ||
         snake_skin_tails[skin][assets_direction_index(DIRECTION_DOWN)] == NULL ||
         snake_skin_tails[skin][assets_direction_index(DIRECTION_LEFT)] == NULL;
}

static void assets_reset(void) {
  snake_head = NULL;
  snake_head_up = NULL;
  snake_head_down = NULL;
  snake_head_left = NULL;
  snake_body = NULL;
  snake_tail = NULL;
  snake_tail_up = NULL;
  snake_tail_down = NULL;
  snake_tail_left = NULL;

  for (uint8_t skin = 0; skin < ASSETS_SNAKE_SKIN_COUNT; skin++) {
    snake_skin_bodies[skin] = NULL;

    for (uint8_t dir = 0; dir < SNAKE_DIR_COUNT; dir++) {
      snake_skin_heads[skin][dir] = NULL;
      snake_skin_tails[skin][dir] = NULL;
    }
  }

  food = NULL;
  frenzy_food = NULL;
  power_up = NULL;
  for (uint8_t i = 0; i < POWERUP_TYPE_COUNT; i++)
    power_up_sprites[i] = NULL;
  background = NULL;
  menu = NULL;
  button = NULL;
  cursor = NULL;
  back_arrow = NULL;
  logo = NULL;
  loaded = false;
}

int assets_load_all(void) {
  if (loaded)
    return 0;

  snake_head = sprite_create((xpm_map_t) snake_head_right_xpm, XPM_INDEXED, 0, true);
  snake_head_up = sprite_create((xpm_map_t) snake_head_up_xpm, XPM_INDEXED, 0, true);
  snake_head_down = sprite_create((xpm_map_t) snake_head_down_xpm, XPM_INDEXED, 0, true);
  snake_head_left = sprite_create((xpm_map_t) snake_head_left_xpm, XPM_INDEXED, 0, true);
  snake_body = sprite_create((xpm_map_t) snake_body_xpm, XPM_INDEXED, 0, true);
  snake_tail = sprite_create((xpm_map_t) snake_tail_right_xpm, XPM_INDEXED, 0, true);
  snake_tail_up = sprite_create((xpm_map_t) snake_tail_up_xpm, XPM_INDEXED, 0, true);
  snake_tail_down = sprite_create((xpm_map_t) snake_tail_down_xpm, XPM_INDEXED, 0, true);
  snake_tail_left = sprite_create((xpm_map_t) snake_tail_left_xpm, XPM_INDEXED, 0, true);

  snake_skin_heads[ASSETS_SNAKE_SKIN_GREEN][assets_direction_index(DIRECTION_RIGHT)] = snake_head;
  snake_skin_heads[ASSETS_SNAKE_SKIN_GREEN][assets_direction_index(DIRECTION_UP)] = snake_head_up;
  snake_skin_heads[ASSETS_SNAKE_SKIN_GREEN][assets_direction_index(DIRECTION_DOWN)] = snake_head_down;
  snake_skin_heads[ASSETS_SNAKE_SKIN_GREEN][assets_direction_index(DIRECTION_LEFT)] = snake_head_left;
  snake_skin_bodies[ASSETS_SNAKE_SKIN_GREEN] = snake_body;
  snake_skin_tails[ASSETS_SNAKE_SKIN_GREEN][assets_direction_index(DIRECTION_RIGHT)] = snake_tail;
  snake_skin_tails[ASSETS_SNAKE_SKIN_GREEN][assets_direction_index(DIRECTION_UP)] = snake_tail_up;
  snake_skin_tails[ASSETS_SNAKE_SKIN_GREEN][assets_direction_index(DIRECTION_DOWN)] = snake_tail_down;
  snake_skin_tails[ASSETS_SNAKE_SKIN_GREEN][assets_direction_index(DIRECTION_LEFT)] = snake_tail_left;

  for (uint8_t skin = ASSETS_SNAKE_SKIN_RED; skin < ASSETS_SNAKE_SKIN_COUNT; skin++) {
    if (assets_load_recolored_skin(skin) != 0) {
      assets_free_all();
      return 1;
    }
  }

  food = sprite_create((xpm_map_t) snake_apple_xpm, XPM_INDEXED, 0, true);
  frenzy_food = sprite_create((xpm_map_t) food_xpm, XPM_INDEXED, 0, true);
  power_up = sprite_create((xpm_map_t) power_up_xpm, XPM_INDEXED, 0, true);
  power_up_sprites[POWERUP_DOUBLE_POINTS] = sprite_create((xpm_map_t) power_up_double_points_xpm, XPM_INDEXED, 0, true);
  power_up_sprites[POWERUP_GHOST] = sprite_create((xpm_map_t) power_up_ghost_xpm, XPM_INDEXED, 0, true);
  power_up_sprites[POWERUP_SHIELD] = sprite_create((xpm_map_t) power_up_shield_xpm, XPM_INDEXED, 0, true);
  power_up_sprites[POWERUP_MAGNET] = sprite_create((xpm_map_t) power_up_magnet_xpm, XPM_INDEXED, 0, true);
  power_up_sprites[POWERUP_FRENZY] = sprite_create((xpm_map_t) power_up_frenzy_xpm, XPM_INDEXED, 0, true);
  power_up_sprites[POWERUP_BONUS_FOOD] = sprite_create((xpm_map_t) power_up_bonus_food_xpm, XPM_INDEXED, 0, true);
  power_up_sprites[POWERUP_SPEED_BOOST] = sprite_create((xpm_map_t) power_up_speed_boost_xpm, XPM_INDEXED, 0, true);
  power_up_sprites[POWERUP_SLOW_ENEMY] = sprite_create((xpm_map_t) power_up_slow_enemy_xpm, XPM_INDEXED, 0, true);
  power_up_sprites[POWERUP_FORCE_GROWTH] = sprite_create((xpm_map_t) power_up_force_growth_xpm, XPM_INDEXED, 0, true);
  background = sprite_create((xpm_map_t) background_xpm, XPM_INDEXED, 0, false);
  menu = sprite_create((xpm_map_t) menu_xpm, XPM_INDEXED, 0, true);
  button = sprite_create((xpm_map_t) button_xpm, XPM_INDEXED, 0, true);
  cursor = sprite_create((xpm_map_t) cursor_xpm, XPM_INDEXED, 0, true);
  back_arrow = sprite_create((xpm_map_t) back_arrow_xpm, XPM_INDEXED, 0, true);
  logo = sprite_create((xpm_map_t) snake_rivals_logo_xpm, XPM_INDEXED, 0, true);

  if (snake_head == NULL ||
      snake_head_up == NULL ||
      snake_head_down == NULL ||
      snake_head_left == NULL ||
      snake_body == NULL ||
      snake_tail == NULL ||
      snake_tail_up == NULL ||
      snake_tail_down == NULL ||
      snake_tail_left == NULL ||
      food == NULL ||
      frenzy_food == NULL ||
      power_up == NULL ||
      power_up_sprites[POWERUP_DOUBLE_POINTS] == NULL ||
      power_up_sprites[POWERUP_GHOST] == NULL ||
      power_up_sprites[POWERUP_SHIELD] == NULL ||
      power_up_sprites[POWERUP_MAGNET] == NULL ||
      power_up_sprites[POWERUP_FRENZY] == NULL ||
      power_up_sprites[POWERUP_BONUS_FOOD] == NULL ||
      power_up_sprites[POWERUP_SPEED_BOOST] == NULL ||
      power_up_sprites[POWERUP_SLOW_ENEMY] == NULL ||
      power_up_sprites[POWERUP_FORCE_GROWTH] == NULL ||
      background == NULL ||
      menu == NULL ||
      button == NULL ||
      cursor == NULL ||
      back_arrow == NULL ||
      logo == NULL) {
    assets_free_all();
    return 1;
  }

  loaded = true;

  return 0;
}

void assets_free_all(void) {
  for (uint8_t skin = ASSETS_SNAKE_SKIN_RED; skin < ASSETS_SNAKE_SKIN_COUNT; skin++) {
    for (uint8_t dir = 0; dir < SNAKE_DIR_COUNT; dir++) {
      sprite_destroy(snake_skin_heads[skin][dir]);
      sprite_destroy(snake_skin_tails[skin][dir]);
    }

    sprite_destroy(snake_skin_bodies[skin]);
  }

  sprite_destroy(snake_head);
  sprite_destroy(snake_head_up);
  sprite_destroy(snake_head_down);
  sprite_destroy(snake_head_left);
  sprite_destroy(snake_body);
  sprite_destroy(snake_tail);
  sprite_destroy(snake_tail_up);
  sprite_destroy(snake_tail_down);
  sprite_destroy(snake_tail_left);
  sprite_destroy(food);
  sprite_destroy(frenzy_food);
  sprite_destroy(power_up);
  for (uint8_t i = 0; i < POWERUP_TYPE_COUNT; i++)
    sprite_destroy(power_up_sprites[i]);
  sprite_destroy(background);
  sprite_destroy(menu);
  sprite_destroy(button);
  sprite_destroy(cursor);
  sprite_destroy(back_arrow);
  sprite_destroy(logo);

  assets_reset();
}

bool assets_are_loaded(void) {
  return loaded;
}

const sprite_t *assets_get_snake_head(void) {
  return snake_head;
}

const sprite_t *assets_get_snake_head_for_direction(direction_t direction) {
  return assets_get_snake_head_for_skin_direction(ASSETS_SNAKE_SKIN_GREEN, direction);
}

const sprite_t *assets_get_snake_head_for_skin_direction(uint8_t skin, direction_t direction) {
  skin = assets_clamp_skin(skin);
  const sprite_t *sprite = snake_skin_heads[skin][assets_direction_index(direction)];
  return sprite != NULL ? sprite : snake_head;
}

const sprite_t *assets_get_red_snake_head_for_direction(direction_t direction) {
  return assets_get_snake_head_for_skin_direction(ASSETS_SNAKE_SKIN_RED, direction);
}

const sprite_t *assets_get_snake_body(void) {
  return snake_body;
}

const sprite_t *assets_get_snake_body_for_skin(uint8_t skin) {
  skin = assets_clamp_skin(skin);
  const sprite_t *sprite = snake_skin_bodies[skin];
  return sprite != NULL ? sprite : snake_body;
}

const sprite_t *assets_get_red_snake_body(void) {
  return assets_get_snake_body_for_skin(ASSETS_SNAKE_SKIN_RED);
}

const sprite_t *assets_get_snake_tail(void) {
  return snake_tail;
}

const sprite_t *assets_get_snake_tail_for_direction(direction_t direction) {
  return assets_get_snake_tail_for_skin_direction(ASSETS_SNAKE_SKIN_GREEN, direction);
}

const sprite_t *assets_get_snake_tail_for_skin_direction(uint8_t skin, direction_t direction) {
  skin = assets_clamp_skin(skin);
  const sprite_t *sprite = snake_skin_tails[skin][assets_direction_index(direction)];
  return sprite != NULL ? sprite : snake_tail;
}

const sprite_t *assets_get_red_snake_tail_for_direction(direction_t direction) {
  return assets_get_snake_tail_for_skin_direction(ASSETS_SNAKE_SKIN_RED, direction);
}

uint32_t assets_get_snake_skin_preview_color(uint8_t skin) {
  switch (assets_clamp_skin(skin)) {
    case ASSETS_SNAKE_SKIN_RED:
      return 0x04;
    case ASSETS_SNAKE_SKIN_BLUE:
      return 0x09;
    case ASSETS_SNAKE_SKIN_PURPLE:
      return 0x0D;
    case ASSETS_SNAKE_SKIN_YELLOW:
      return 0x0E;
    case ASSETS_SNAKE_SKIN_GREEN:
    default:
      return 0x0A;
  }
}

const char *assets_get_snake_skin_name(uint8_t skin) {
  switch (assets_clamp_skin(skin)) {
    case ASSETS_SNAKE_SKIN_RED:
      return "RED";
    case ASSETS_SNAKE_SKIN_BLUE:
      return "BLUE";
    case ASSETS_SNAKE_SKIN_PURPLE:
      return "PURPLE";
    case ASSETS_SNAKE_SKIN_YELLOW:
      return "YELLOW";
    case ASSETS_SNAKE_SKIN_GREEN:
    default:
      return "GREEN";
  }
}

const sprite_t *assets_get_food(void) {
  return food;
}

const sprite_t *assets_get_frenzy_food(void) {
  return frenzy_food != NULL ? frenzy_food : food;
}

const sprite_t *assets_get_power_up(void) {
  return power_up;
}

const sprite_t *assets_get_power_up_for_type(powerup_type_t type) {
  if (type >= POWERUP_TYPE_COUNT)
    return power_up;

  if (power_up_sprites[type] == NULL)
    return power_up;

  return power_up_sprites[type];
}

const sprite_t *assets_get_background(void) {
  return background;
}

const sprite_t *assets_get_menu(void) {
  return menu;
}

const sprite_t *assets_get_button(void) {
  return button;
}

const sprite_t *assets_get_cursor(void) {
  return cursor;
}

const sprite_t *assets_get_back_arrow(void) {
  return back_arrow;
}

const sprite_t *assets_get_logo(void) {
  return logo;
}
