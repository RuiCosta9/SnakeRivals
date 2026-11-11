/**
 * @file renderer_arena_battle.c
 * @brief Arena Battle multiplayer mode rendering with camera-space helpers.
 */

#include "renderer_internal.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "assets.h"
#include "arena_walls.h"
#include "food.h"
#include "multiplayer_session.h"
#include "snake.h"
#include "text.h"
#include "ui_layout.h"
#include "vbe.h"

#define ARENA_TOP_UI_HEIGHT 64

int32_t renderer_clamp_i32(int32_t value, int32_t min_value, int32_t max_value) {
  if (value < min_value)
    return min_value;

  if (value > max_value)
    return max_value;

  return value;
}

static uint16_t renderer_arena_interpolate_u16(uint16_t start, uint16_t end, uint16_t step, uint16_t steps) {
  if (steps == 0 || step >= steps)
    return end;

  int16_t delta = (int16_t) end - (int16_t) start;
  return (uint16_t) ((int16_t) start + (delta * (int16_t) step) / (int16_t) steps);
}


static int32_t renderer_arena_interpolated_food_world_axis(uint8_t player_id, uint16_t cell_size, bool horizontal) {
  uint8_t count = multiplayer_session_get_food_animation_position_count(player_id);
  uint8_t step = multiplayer_session_get_animation_step();
  uint8_t steps = multiplayer_session_get_animation_steps();

  if (count <= 1 || steps == 0 || step >= steps) {
    position_t position = multiplayer_session_get_food_animation_position(player_id, count > 0 ? (uint8_t) (count - 1) : 0);
    return (int32_t) (horizontal ? position.x : position.y) * cell_size;
  }

  uint8_t segment_count = (uint8_t) (count - 1);
  uint16_t scaled_step = (uint16_t) step * segment_count;
  uint8_t segment = (uint8_t) (scaled_step / steps);
  uint8_t segment_step = (uint8_t) (scaled_step % steps);

  if (segment >= segment_count) {
    position_t position = multiplayer_session_get_food_animation_position(player_id, (uint8_t) (count - 1));
    return (int32_t) (horizontal ? position.x : position.y) * cell_size;
  }

  position_t start = multiplayer_session_get_food_animation_position(player_id, segment);
  position_t end = multiplayer_session_get_food_animation_position(player_id, (uint8_t) (segment + 1));
  uint16_t start_pixel = (uint16_t) (horizontal ? start.x : start.y) * cell_size;
  uint16_t end_pixel = (uint16_t) (horizontal ? end.x : end.y) * cell_size;

  return renderer_arena_interpolate_u16(start_pixel, end_pixel, segment_step, steps);
}

static int32_t renderer_arena_interpolate_i32(int32_t start, int32_t end, uint16_t step, uint16_t steps) {
  if (steps == 0 || step >= steps)
    return end;

  return start + ((end - start) * (int32_t) step) / (int32_t) steps;
}

static int32_t renderer_arena_interpolated_head_pixel_x(const snake_t *snake, uint8_t player_id, uint16_t cell_size) {
  position_t current = snake_get_head(snake);
  position_t previous = multiplayer_session_get_previous_snake_segment(player_id, 0);
  uint8_t step = multiplayer_session_get_animation_step();
  uint8_t steps = multiplayer_session_get_animation_steps();
  int32_t previous_x = (int32_t) previous.x * cell_size + cell_size / 2;
  int32_t current_x = (int32_t) current.x * cell_size + cell_size / 2;

  return renderer_arena_interpolate_i32(previous_x, current_x, step, steps);
}

static int32_t renderer_arena_interpolated_head_pixel_y(const snake_t *snake, uint8_t player_id, uint16_t cell_size) {
  position_t current = snake_get_head(snake);
  position_t previous = multiplayer_session_get_previous_snake_segment(player_id, 0);
  uint8_t step = multiplayer_session_get_animation_step();
  uint8_t steps = multiplayer_session_get_animation_steps();
  int32_t previous_y = (int32_t) previous.y * cell_size + cell_size / 2;
  int32_t current_y = (int32_t) current.y * cell_size + cell_size / 2;

  return renderer_arena_interpolate_i32(previous_y, current_y, step, steps);
}

static int renderer_submit_snake_camera_offset(const snake_t *snake, uint8_t player_id, int32_t camera_x, int32_t camera_y, uint16_t cell_size, uint16_t y_offset, uint16_t viewport_h, int16_t body_layer, int16_t head_layer) {
  if (snake == NULL)
    return 0;

  uint8_t skin = multiplayer_session_get_player_skin(player_id);
  const sprite_t *head = assets_get_snake_head_for_skin_direction(skin, renderer_get_snake_head_direction(snake));
  const sprite_t *body = assets_get_snake_body_for_skin(skin);
  const sprite_t *tail = assets_get_snake_tail_for_skin_direction(skin, renderer_get_snake_tail_direction(snake));
  uint16_t hres = vbe_get_hres();
  uint16_t length = snake_get_length(snake);
  uint16_t previous_length = multiplayer_session_get_previous_snake_length(player_id);
  uint8_t step = multiplayer_session_get_animation_step();
  uint8_t steps = multiplayer_session_get_animation_steps();

  for (uint16_t i = length; i > 0; i--) {
    uint16_t index = i - 1;
    position_t segment = snake_get_segment(snake, index);
    position_t previous = index < previous_length ? multiplayer_session_get_previous_snake_segment(player_id, index) : segment;
    uint16_t current_x = (uint16_t) segment.x * cell_size;
    uint16_t current_y = (uint16_t) segment.y * cell_size;
    uint16_t previous_x = (uint16_t) previous.x * cell_size;
    uint16_t previous_y = (uint16_t) previous.y * cell_size;
    int32_t world_x = renderer_arena_interpolate_u16(previous_x, current_x, step, steps);
    int32_t world_y = renderer_arena_interpolate_u16(previous_y, current_y, step, steps);
    int32_t screen_x = world_x - camera_x;
    int32_t screen_y = world_y - camera_y + y_offset;

    if (screen_x < 0 || screen_y < (int32_t) y_offset)
      continue;

    if (screen_x + cell_size > hres || screen_y + cell_size > y_offset + viewport_h)
      continue;

    const sprite_t *sprite = index == 0 ? head : (index == length - 1 ? tail : body);
    int16_t layer = index == 0 ? head_layer : body_layer + (int16_t) index;

    if (renderer_submit_sprite(sprite, (uint16_t) screen_x, (uint16_t) screen_y, layer) != 0)
      return 1;
  }

  return 0;
}

int renderer_submit_snake_camera(const snake_t *snake, int32_t camera_x, int32_t camera_y, uint16_t cell_size, int16_t body_layer, int16_t head_layer) {
  return renderer_submit_snake_camera_offset(snake, 1, camera_x, camera_y, cell_size, 0, vbe_get_vres(), body_layer, head_layer);
}

static int renderer_submit_item_camera_offset(const food_t *item, int32_t camera_x, int32_t camera_y, uint16_t cell_size, uint16_t y_offset, uint16_t viewport_h, uint32_t color, int16_t z, bool powerup) {
  if (item == NULL || !food_is_active(item))
    return 0;

  uint16_t hres = vbe_get_hres();
  position_t position = food_get_position(item);
  int32_t screen_x = (int32_t) position.x * cell_size - camera_x;
  int32_t screen_y = (int32_t) position.y * cell_size - camera_y + y_offset;

  if (screen_x < 0 || screen_y < (int32_t) y_offset)
    return 0;

  if (screen_x + cell_size > hres || screen_y + cell_size > y_offset + viewport_h)
    return 0;

  if (powerup)
    return renderer_submit_powerup_icon(POWERUP_FORCE_GROWTH, (uint16_t) screen_x, (uint16_t) screen_y, cell_size, z);

  if (color == 0x04) {
    const sprite_t *food_sprite = assets_get_food();

    if (food_sprite != NULL)
      return renderer_submit_sprite(food_sprite, (uint16_t) screen_x, (uint16_t) screen_y, z);
  }

  return renderer_submit_rect((uint16_t) screen_x, (uint16_t) screen_y, cell_size, cell_size, color, z);
}

int renderer_submit_item_camera(const food_t *item, int32_t camera_x, int32_t camera_y, uint16_t cell_size, uint32_t color, int16_t z) {
  return renderer_submit_item_camera_offset(item, camera_x, camera_y, cell_size, 0, vbe_get_vres(), color, z, false);
}

static int renderer_submit_player_item_camera(uint8_t player_id, int32_t camera_x, int32_t camera_y, uint16_t cell_size, uint16_t y_offset, uint16_t viewport_h, bool powerup) {
  bool active = powerup ? multiplayer_session_player_powerup_active(player_id) : multiplayer_session_player_food_active(player_id);

  if (!active)
    return 0;

  uint16_t hres = vbe_get_hres();
  position_t position = powerup ? multiplayer_session_get_player_powerup_position(player_id) : multiplayer_session_get_player_food_position(player_id);
  int32_t world_x = powerup ? (int32_t) position.x * cell_size : renderer_arena_interpolated_food_world_axis(player_id, cell_size, true);
  int32_t world_y = powerup ? (int32_t) position.y * cell_size : renderer_arena_interpolated_food_world_axis(player_id, cell_size, false);
  int32_t screen_x = world_x - camera_x;
  int32_t screen_y = world_y - camera_y + y_offset;

  if (screen_x < 0 || screen_y < (int32_t) y_offset)
    return 0;

  if (screen_x + cell_size > hres || screen_y + cell_size > y_offset + viewport_h)
    return 0;

  if (powerup)
    return renderer_submit_powerup_icon(multiplayer_session_get_player_powerup_type(player_id), (uint16_t) screen_x, (uint16_t) screen_y, cell_size, RENDER_LAYER_POWER_UP);

  const sprite_t *food_sprite = assets_get_food();

  if (food_sprite != NULL)
    return renderer_submit_sprite(food_sprite, (uint16_t) screen_x, (uint16_t) screen_y, RENDER_LAYER_FOOD);

  return renderer_submit_rect((uint16_t) screen_x, (uint16_t) screen_y, cell_size, cell_size, 0x04, RENDER_LAYER_FOOD);
}

static int renderer_submit_world_border_camera_offset(int32_t camera_x, int32_t camera_y, uint16_t world_w, uint16_t world_h, uint16_t cell_size, uint16_t y_offset, uint16_t viewport_h) {
  uint16_t hres = vbe_get_hres();

  if (camera_y <= 0) {
    if (renderer_submit_rect(0, y_offset, hres, cell_size, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if (camera_x <= 0) {
    if (renderer_submit_rect(0, y_offset, cell_size, viewport_h, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if (camera_y + viewport_h >= world_h) {
    uint16_t y = world_h > (uint16_t) camera_y + cell_size ? (uint16_t) (world_h - camera_y - cell_size) : 0;

    if (renderer_submit_rect(0, y_offset + y, hres, cell_size, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if (camera_x + hres >= world_w) {
    uint16_t x = world_w > (uint16_t) camera_x + cell_size ? (uint16_t) (world_w - camera_x - cell_size) : 0;

    if (renderer_submit_rect(x, y_offset, cell_size, viewport_h, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  return 0;
}

int renderer_submit_world_border_camera(int32_t camera_x, int32_t camera_y, uint16_t world_w, uint16_t world_h, uint16_t cell_size) {
  return renderer_submit_world_border_camera_offset(camera_x, camera_y, world_w, world_h, cell_size, 0, vbe_get_vres());
}

#define ARENA_WALL_FRAME_COLOR 0x08  /* moldura (igual a borda externa) */
#define ARENA_WALL_FILL_COLOR  0x07  /* miolo mais claro                */
#define ARENA_WALL_INSET       3     /* espessura da moldura, em pixels */

static int renderer_clip_submit_world_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, int32_t view_top, int32_t view_bottom, uint16_t hres) {
  int32_t left = x < 0 ? 0 : x;
  int32_t top = y < view_top ? view_top : y;
  int32_t right = x + w;
  int32_t bottom = y + h;

  if (right > (int32_t) hres)
    right = (int32_t) hres;

  if (bottom > view_bottom)
    bottom = view_bottom;

  if (right <= left || bottom <= top)
    return 0;

  return renderer_submit_rect((uint16_t) left, (uint16_t) top, (uint16_t) (right - left), (uint16_t) (bottom - top), color, RENDER_LAYER_WORLD);
}

int renderer_submit_arena_walls_camera(int32_t camera_x, int32_t camera_y, uint16_t cell_size, uint16_t y_offset, uint16_t viewport_h) {
  uint16_t hres = vbe_get_hres();
  uint8_t block_count = arena_walls_get_count();
  int32_t view_top = (int32_t) y_offset;
  int32_t view_bottom = (int32_t) y_offset + (int32_t) viewport_h;
  const int32_t inset = ARENA_WALL_INSET;

  for (uint8_t b = 0; b < block_count; b++) {
    int16_t block_x = 0, block_y = 0, block_w = 0, block_h = 0;

    arena_walls_get_block(b, &block_x, &block_y, &block_w, &block_h);

    int32_t bx = (int32_t) block_x * cell_size - camera_x;
    int32_t by = (int32_t) block_y * cell_size - camera_y + (int32_t) y_offset;
    int32_t bw = (int32_t) block_w * cell_size;
    int32_t bh = (int32_t) block_h * cell_size;

    if (renderer_clip_submit_world_rect(bx, by, bw, bh, ARENA_WALL_FRAME_COLOR, view_top, view_bottom, hres) != 0)
      return 1;

    if (bw > 2 * inset && bh > 2 * inset) {
      if (renderer_clip_submit_world_rect(bx + inset, by + inset, bw - 2 * inset, bh - 2 * inset, ARENA_WALL_FILL_COLOR, view_top, view_bottom, hres) != 0)
        return 1;
    }
  }

  return 0;
}

int renderer_draw_multiplayer_arena(uint16_t hres, uint16_t vres, multiplayer_role_t role, const snake_t *player1, const snake_t *player2) {
  uint16_t grid_columns = multiplayer_session_get_grid_columns();
  uint16_t grid_rows = multiplayer_session_get_grid_rows();
  uint16_t cell_size = MULTIPLAYER_CELL_SIZE;
  uint16_t world_w = grid_columns * cell_size;
  uint16_t world_h = grid_rows * cell_size;
  uint16_t viewport_y = ARENA_TOP_UI_HEIGHT;
  uint16_t viewport_h = vres > viewport_y ? vres - viewport_y : vres;
  const snake_t *local = role == MP_ROLE_HOST ? player1 : player2;
  const snake_t *enemy = role == MP_ROLE_HOST ? player2 : player1;
  uint8_t local_id = role == MP_ROLE_HOST ? 1 : 2;
  uint8_t enemy_id = local_id == 1 ? 2 : 1;
  int32_t local_head_pixel_x = renderer_arena_interpolated_head_pixel_x(local, local_id, cell_size);
  int32_t local_head_pixel_y = renderer_arena_interpolated_head_pixel_y(local, local_id, cell_size);
  int32_t camera_x = local_head_pixel_x - (int32_t) hres / 2;
  int32_t camera_y = local_head_pixel_y - (int32_t) viewport_h / 2;
  int32_t max_camera_x = world_w > hres ? (int32_t) world_w - hres : 0;
  int32_t max_camera_y = world_h > viewport_h ? (int32_t) world_h - viewport_h : 0;
  char info_text[80];

  camera_x = renderer_clamp_i32(camera_x, 0, max_camera_x);
  camera_y = renderer_clamp_i32(camera_y, 0, max_camera_y);

  if (renderer_submit_rect(0, 0, hres, ARENA_TOP_UI_HEIGHT, 0x00, RENDER_LAYER_UI + 5) != 0)
    return 1;

  snprintf(info_text, sizeof(info_text), "ARENA BATTLE   %s   YOU %u   OPP %u   LEN %u",
           role == MP_ROLE_HOST ? "PLAYER 1" : "PLAYER 2",
           (unsigned int) multiplayer_session_get_player_score(local_id),
           (unsigned int) multiplayer_session_get_player_score(enemy_id),
           (unsigned int) snake_get_length(local));

  if (renderer_submit_text(info_text, 20, 20, 2, 0x0F, RENDER_LAYER_UI + 6) != 0)
    return 1;

  if (renderer_submit_multiplayer_inventory(local_id, hres > 140 ? hres - 140 : 20, 10) != 0)
    return 1;

  if (renderer_submit_play_background(0, viewport_y, hres, viewport_h, camera_x, camera_y) != 0)
    return 1;

  if (renderer_submit_world_border_camera_offset(camera_x, camera_y, world_w, world_h, cell_size, viewport_y, viewport_h) != 0)
    return 1;

  if (renderer_submit_arena_walls_camera(camera_x, camera_y, cell_size, viewport_y, viewport_h) != 0)
    return 1;

  for (uint8_t item_player = 1; item_player <= 2; item_player++) {
    if (renderer_submit_player_item_camera(item_player, camera_x, camera_y, cell_size, viewport_y, viewport_h, false) != 0)
      return 1;

    if (renderer_submit_player_item_camera(item_player, camera_x, camera_y, cell_size, viewport_y, viewport_h, true) != 0)
      return 1;
  }

  if (snake_is_alive(local)) {
    if (renderer_submit_snake_camera_offset(local, local_id, camera_x, camera_y, cell_size, viewport_y, viewport_h, RENDER_LAYER_SNAKE_BODY, RENDER_LAYER_SNAKE_HEAD) != 0)
      return 1;
  }
  else {
    if (renderer_submit_death_snake_camera_skin(local, multiplayer_session_get_player_skin(local_id), camera_x, camera_y, cell_size, viewport_y, viewport_h, multiplayer_session_get_player_death_animation_tick(local_id), multiplayer_session_get_death_animation_ticks(), RENDER_LAYER_SNAKE_BODY, RENDER_LAYER_SNAKE_HEAD) != 0)
      return 1;
  }

  if (snake_is_alive(enemy)) {
    if (renderer_submit_snake_camera_offset(enemy, enemy_id, camera_x, camera_y, cell_size, viewport_y, viewport_h, RENDER_LAYER_SNAKE_BODY, RENDER_LAYER_SNAKE_HEAD) != 0)
      return 1;
  }
  else {
    if (renderer_submit_death_snake_camera_skin(enemy, multiplayer_session_get_player_skin(enemy_id), camera_x, camera_y, cell_size, viewport_y, viewport_h, multiplayer_session_get_player_death_animation_tick(enemy_id), multiplayer_session_get_death_animation_ticks(), RENDER_LAYER_SNAKE_BODY, RENDER_LAYER_SNAKE_HEAD) != 0)
      return 1;
  }

  return renderer_draw_multiplayer_result_overlay(hres, vres);
}
