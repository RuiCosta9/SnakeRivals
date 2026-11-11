/**
 * @file renderer_score_race.c
 * @brief Score Race multiplayer mode rendering.
 */

#include "renderer_internal.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "assets.h"
#include "food.h"
#include "multiplayer_session.h"
#include "snake.h"
#include "text.h"
#include "ui_layout.h"
#include "vbe.h"

#define SCORE_RACE_PREVIEW_W 220
#define SCORE_RACE_PREVIEW_H 170
#define SCORE_RACE_PREVIEW_MARGIN 16

static uint16_t renderer_score_interpolate_u16(uint16_t start, uint16_t end, uint16_t step, uint16_t steps) {
  if (steps == 0 || step >= steps)
    return end;

  int16_t delta = (int16_t) end - (int16_t) start;
  return (uint16_t) ((int16_t) start + (delta * step) / steps);
}


static uint16_t renderer_score_interpolated_food_axis(uint8_t player_id, uint16_t arena_origin, uint16_t cell_size, bool horizontal) {
  uint8_t count = multiplayer_session_get_food_animation_position_count(player_id);
  uint8_t step = multiplayer_session_get_animation_step();
  uint8_t steps = multiplayer_session_get_animation_steps();

  if (count <= 1 || steps == 0 || step >= steps) {
    position_t position = multiplayer_session_get_food_animation_position(player_id, count > 0 ? (uint8_t) (count - 1) : 0);
    return arena_origin + (uint16_t) (horizontal ? position.x : position.y) * cell_size;
  }

  uint8_t segment_count = (uint8_t) (count - 1);
  uint16_t scaled_step = (uint16_t) step * segment_count;
  uint8_t segment = (uint8_t) (scaled_step / steps);
  uint8_t segment_step = (uint8_t) (scaled_step % steps);

  if (segment >= segment_count) {
    position_t position = multiplayer_session_get_food_animation_position(player_id, (uint8_t) (count - 1));
    return arena_origin + (uint16_t) (horizontal ? position.x : position.y) * cell_size;
  }

  position_t start = multiplayer_session_get_food_animation_position(player_id, segment);
  position_t end = multiplayer_session_get_food_animation_position(player_id, (uint8_t) (segment + 1));
  uint16_t start_pixel = arena_origin + (uint16_t) (horizontal ? start.x : start.y) * cell_size;
  uint16_t end_pixel = arena_origin + (uint16_t) (horizontal ? end.x : end.y) * cell_size;

  return renderer_score_interpolate_u16(start_pixel, end_pixel, segment_step, steps);
}

static int renderer_submit_multiplayer_snake_interpolated(const snake_t *snake, uint8_t player_id, uint16_t arena_x, uint16_t arena_y, uint16_t cell_size, int16_t body_layer, int16_t head_layer) {
  if (snake == NULL)
    return 0;

  uint8_t skin = multiplayer_session_get_player_skin(player_id);
  const sprite_t *head = assets_get_snake_head_for_skin_direction(skin, renderer_get_snake_head_direction(snake));
  const sprite_t *body = assets_get_snake_body_for_skin(skin);
  const sprite_t *tail = assets_get_snake_tail_for_skin_direction(skin, renderer_get_snake_tail_direction(snake));
  uint16_t length = snake_get_length(snake);
  uint16_t previous_length = multiplayer_session_get_previous_snake_length(player_id);
  uint8_t step = multiplayer_session_get_animation_step();
  uint8_t steps = multiplayer_session_get_animation_steps();

  for (uint16_t i = length; i > 0; i--) {
    uint16_t index = i - 1;
    position_t segment = snake_get_segment(snake, index);
    position_t previous = index < previous_length ? multiplayer_session_get_previous_snake_segment(player_id, index) : segment;
    uint16_t current_x = arena_x + (uint16_t) segment.x * cell_size;
    uint16_t current_y = arena_y + (uint16_t) segment.y * cell_size;
    uint16_t previous_x = arena_x + (uint16_t) previous.x * cell_size;
    uint16_t previous_y = arena_y + (uint16_t) previous.y * cell_size;
    uint16_t x = renderer_score_interpolate_u16(previous_x, current_x, step, steps);
    uint16_t y = renderer_score_interpolate_u16(previous_y, current_y, step, steps);

    if (index == 0) {
      if (renderer_submit_sprite(head, x, y, head_layer) != 0)
        return 1;
    }
    else if (index == length - 1) {
      if (renderer_submit_sprite(tail, x, y, body_layer + (int16_t) index) != 0)
        return 1;
    }
    else {
      if (renderer_submit_sprite(body, x, y, body_layer + (int16_t) index) != 0)
        return 1;
    }
  }

  return 0;
}

static int renderer_submit_snake_minimap_scaled(const snake_t *snake, uint16_t map_x, uint16_t map_y, uint16_t map_w, uint16_t map_h, uint16_t grid_columns, uint16_t grid_rows, uint32_t color, int16_t z) {
  if (snake == NULL || grid_columns == 0 || grid_rows == 0)
    return 0;

  uint16_t length = snake_get_length(snake);

  for (uint16_t i = length; i > 0; i--) {
    uint16_t index = i - 1;
    position_t segment = snake_get_segment(snake, index);
    uint16_t x = map_x + (uint16_t) (((uint32_t) segment.x * map_w) / grid_columns);
    uint16_t y = map_y + (uint16_t) (((uint32_t) segment.y * map_h) / grid_rows);
    uint16_t next_x = map_x + (uint16_t) (((uint32_t) (segment.x + 1) * map_w) / grid_columns);
    uint16_t next_y = map_y + (uint16_t) (((uint32_t) (segment.y + 1) * map_h) / grid_rows);
    uint16_t w = next_x > x ? next_x - x : 2;
    uint16_t h = next_y > y ? next_y - y : 2;

    if (index == 0) {
      if (w < 4) w = 4;
      if (h < 4) h = 4;
    }

    if (renderer_submit_rect(x, y, w, h, color, z + (int16_t) index) != 0)
      return 1;
  }

  return 0;
}

int renderer_submit_snake_minimap(const snake_t *snake, uint16_t panel_x, uint16_t panel_y, uint16_t cell_size, uint32_t color, int16_t z) {
  uint16_t map_w = multiplayer_session_get_grid_columns() * cell_size;
  uint16_t map_h = multiplayer_session_get_grid_rows() * cell_size;
  return renderer_submit_snake_minimap_scaled(snake, panel_x, panel_y, map_w, map_h, multiplayer_session_get_grid_columns(), multiplayer_session_get_grid_rows(), color, z);
}

int renderer_submit_score_mode_minimap(const snake_t *opponent, uint16_t hres) {
  char score_text[24];
  uint16_t panel_w = SCORE_RACE_PREVIEW_W;
  uint16_t panel_h = SCORE_RACE_PREVIEW_H;
  uint16_t panel_x = hres > panel_w + SCORE_RACE_PREVIEW_MARGIN ? hres - panel_w - SCORE_RACE_PREVIEW_MARGIN : 8;
  uint16_t panel_y = 16;
  uint16_t map_x = panel_x + 10;
  uint16_t map_y = panel_y + 42;
  uint16_t map_w = panel_w - 20;
  uint16_t map_h = panel_h - 52;
  uint8_t opponent_id = multiplayer_session_get_role() == MP_ROLE_HOST ? 2 : 1;

  snprintf(score_text, sizeof(score_text), "OPP SCORE %u", (unsigned int) multiplayer_session_get_player_score(opponent_id));

  if (renderer_submit_rect(panel_x + 2, panel_y + 2, panel_w, panel_h, 0x00, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_rect(panel_x, panel_y, panel_w, panel_h, 0x08, RENDER_LAYER_UI + 1) != 0)
    return 1;

  if (renderer_submit_rect(map_x, map_y, map_w, map_h, 0x01, RENDER_LAYER_UI + 2) != 0)
    return 1;

  if (renderer_submit_text("OPPONENT", panel_x + 10, panel_y + 6, 2, 0x0F, RENDER_LAYER_UI + 3) != 0)
    return 1;

  if (renderer_submit_text(score_text, panel_x + 10, panel_y + 24, 1, 0x0E, RENDER_LAYER_UI + 3) != 0)
    return 1;

  if (renderer_submit_snake_minimap_scaled(opponent, map_x, map_y, map_w, map_h, multiplayer_session_get_grid_columns(), multiplayer_session_get_grid_rows(), assets_get_snake_skin_preview_color(multiplayer_session_get_player_skin(opponent_id)), RENDER_LAYER_UI + 4) != 0)
    return 1;

  return 0;
}

int renderer_submit_multiplayer_player_food(uint8_t player_id, uint16_t arena_x, uint16_t arena_y) {
  if (!multiplayer_session_player_food_active(player_id))
    return 0;

  uint16_t food_x = renderer_score_interpolated_food_axis(player_id, arena_x, MULTIPLAYER_CELL_SIZE, true);
  uint16_t food_y = renderer_score_interpolated_food_axis(player_id, arena_y, MULTIPLAYER_CELL_SIZE, false);
  const sprite_t *food_sprite = assets_get_food();

  if (food_sprite != NULL)
    return renderer_submit_sprite(food_sprite, food_x, food_y, RENDER_LAYER_FOOD);

  return renderer_submit_rect(food_x, food_y, MULTIPLAYER_CELL_SIZE, MULTIPLAYER_CELL_SIZE, 0x04, RENDER_LAYER_FOOD);
}

int renderer_submit_multiplayer_player_powerup(uint8_t player_id, uint16_t arena_x, uint16_t arena_y) {
  if (!multiplayer_session_player_powerup_active(player_id))
    return 0;

  position_t powerup_position = multiplayer_session_get_player_powerup_position(player_id);
  uint16_t powerup_x = arena_x + (uint16_t) powerup_position.x * MULTIPLAYER_CELL_SIZE;
  uint16_t powerup_y = arena_y + (uint16_t) powerup_position.y * MULTIPLAYER_CELL_SIZE;

  return renderer_submit_powerup_icon(multiplayer_session_get_player_powerup_type(player_id), powerup_x, powerup_y, MULTIPLAYER_CELL_SIZE, RENDER_LAYER_POWER_UP);
}

int renderer_submit_powerup_inventory_slot(uint16_t x, uint16_t y, uint16_t size, const char *label, bool filled, powerup_type_t type, int16_t z) {
  if (renderer_submit_rect(x + 3, y + 3, size, size, 0x00, z) != 0)
    return 1;

  if (renderer_submit_rect(x, y, size, size, 0x08, z + 1) != 0)
    return 1;

  if (size > 8) {
    if (renderer_submit_rect(x + 4, y + 4, size - 8, size - 8, 0x01, z + 2) != 0)
      return 1;
  }

  if (filled) {
    if (renderer_submit_powerup_icon(type, x + 8, y + 8, size > 16 ? size - 16 : size, z + 3) != 0)
      return 1;
  }
  else {
    if (renderer_submit_centered_text("--", x + size / 2, y + size / 2 - TEXT_FONT_HEIGHT / 2, 1, 0x0F, z + 3) != 0)
      return 1;
  }

  return renderer_submit_centered_text(label, x + size / 2, y + size + 6, 1, 0x0E, z + 3);
}

static int renderer_submit_powerup_status_line(const char *label, uint16_t ticks, uint16_t x, uint16_t y, uint32_t color, int16_t z) {
  char text[24];

  if (label == NULL || label[0] == '\0')
    return 0;

  if (ticks > 0)
    snprintf(text, sizeof(text), "%s %u", label, (unsigned) ((ticks + 59) / 60));
  else
    snprintf(text, sizeof(text), "%s", label);

  return renderer_submit_text(text, x, y, 2, color, z);
}

int renderer_submit_multiplayer_inventory(uint8_t player_id, uint16_t x, uint16_t y) {
  uint16_t hres = vbe_get_hres();
  uint16_t slot = 50;
  uint16_t gap = 14;
  uint16_t pair_w = (uint16_t) (slot * 2 + gap);
  bool held = multiplayer_session_player_has_held_powerup(player_id);
  bool next = multiplayer_session_player_has_queued_powerup(player_id);
  powerup_type_t next_type = next ? multiplayer_session_get_player_queued_powerup(player_id) : POWERUP_DOUBLE_POINTS;
  uint16_t status_x = (uint16_t) (x + pair_w + 12);
  uint16_t status_y = (uint16_t) (y + 4);

  if ((uint32_t) status_x + 150 > hres && hres > pair_w + 170) {
    x = (uint16_t) (hres - pair_w - 150);
    status_x = (uint16_t) (x + pair_w + 12);
  }

  if (renderer_submit_powerup_inventory_slot(x, y, slot, "ITEM", held, multiplayer_session_get_player_held_powerup(player_id), RENDER_LAYER_UI + 5) != 0)
    return 1;

  if (renderer_submit_powerup_inventory_slot((uint16_t) (x + slot + gap), y, slot, "NEXT", next, next_type, RENDER_LAYER_UI + 5) != 0)
    return 1;

  if (multiplayer_session_player_has_shield(player_id)) {
    if (renderer_submit_powerup_status_line("SHIELD", 0, status_x, status_y, 0x0A, RENDER_LAYER_UI + 8) != 0)
      return 1;
    status_y = (uint16_t) (status_y + TEXT_FONT_HEIGHT * 2 + 4);
  }

  uint8_t effect_count = multiplayer_session_get_player_effect_count(player_id);
  for (uint8_t i = 0; i < effect_count && i < MULTIPLAYER_EFFECT_SLOTS; i++) {
    powerup_type_t type = multiplayer_session_get_player_effect_type(player_id, i);

    if (renderer_submit_powerup_status_line(renderer_powerup_label(type), multiplayer_session_get_player_effect_ticks(player_id, i), status_x, status_y, 0x0E, RENDER_LAYER_UI + 8) != 0)
      return 1;

    status_y = (uint16_t) (status_y + TEXT_FONT_HEIGHT * 2 + 4);
  }

  return 0;
}

int renderer_submit_multiplayer_scores(uint16_t arena_x, uint16_t arena_y, uint16_t arena_w) {
  char local_text[28];
  char opponent_text[28];
  uint8_t local_id = multiplayer_session_get_role() == MP_ROLE_HOST ? 1 : 2;
  uint8_t opponent_id = local_id == 1 ? 2 : 1;

  snprintf(local_text, sizeof(local_text), "YOU %u", (unsigned int) multiplayer_session_get_player_score(local_id));
  snprintf(opponent_text, sizeof(opponent_text), "OPP %u", (unsigned int) multiplayer_session_get_player_score(opponent_id));

  if (renderer_submit_text(local_text, arena_x, arena_y >= 32 ? arena_y - 32 : 8, 2, 0x0A, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_text(opponent_text, arena_x + arena_w / 2, arena_y >= 32 ? arena_y - 32 : 8, 2, 0x0B, RENDER_LAYER_UI) != 0)
    return 1;

  return 0;
}

int renderer_draw_multiplayer_score(uint16_t hres, uint16_t vres, multiplayer_role_t role, const snake_t *player1, const snake_t *player2) {
  uint16_t grid_columns = multiplayer_session_get_grid_columns();
  uint16_t grid_rows = multiplayer_session_get_grid_rows();
  uint16_t arena_w = grid_columns * MULTIPLAYER_CELL_SIZE;
  uint16_t arena_h = grid_rows * MULTIPLAYER_CELL_SIZE;
  uint16_t preview_reserved_w = SCORE_RACE_PREVIEW_W + 2 * SCORE_RACE_PREVIEW_MARGIN;
  uint16_t available_w = hres > preview_reserved_w ? hres - preview_reserved_w : hres;
  uint16_t arena_x = available_w > arena_w ? (available_w - arena_w) / 2 : 0;
  uint16_t arena_y = vres > arena_h + 64 ? (vres - arena_h + 32) / 2 : 64;
  uint16_t border = MULTIPLAYER_CELL_SIZE;
  const snake_t *local = role == MP_ROLE_HOST ? player1 : player2;
  const snake_t *opponent = role == MP_ROLE_HOST ? player2 : player1;
  uint8_t local_id = role == MP_ROLE_HOST ? 1 : 2;

  if (renderer_submit_play_background(arena_x, arena_y, arena_w, arena_h, 0, 0) != 0)
    return 1;

  if (arena_x >= border) {
    if (renderer_submit_rect(arena_x - border, arena_y, border, arena_h, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if ((uint32_t) arena_x + arena_w + border <= available_w) {
    if (renderer_submit_rect(arena_x + arena_w, arena_y, border, arena_h, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if (arena_y >= border) {
    uint16_t x = arena_x >= border ? arena_x - border : arena_x;
    uint16_t w = arena_x >= border ? arena_w + 2 * border : arena_w;

    if (renderer_submit_rect(x, arena_y - border, w, border, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if ((uint32_t) arena_y + arena_h + border <= vres) {
    uint16_t x = arena_x >= border ? arena_x - border : arena_x;
    uint16_t w = arena_x >= border ? arena_w + 2 * border : arena_w;

    if (renderer_submit_rect(x, arena_y + arena_h, w, border, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if (renderer_submit_text(role == MP_ROLE_HOST ? "PLAYER 1" : "PLAYER 2", arena_x, 16, 3, 0x0F, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_text("SCORE RACE", arena_x, 44, 2, 0x0E, RENDER_LAYER_UI) != 0)
    return 1;

  if (renderer_submit_multiplayer_scores(arena_x, arena_y, arena_w) != 0)
    return 1;

  if (renderer_submit_multiplayer_inventory(local_id, arena_x + arena_w - 116, 8) != 0)
    return 1;

  if (renderer_submit_multiplayer_player_food(local_id, arena_x, arena_y) != 0)
    return 1;

  if (renderer_submit_multiplayer_player_powerup(local_id, arena_x, arena_y) != 0)
    return 1;

  if (snake_is_alive(local)) {
    if (renderer_submit_multiplayer_snake_interpolated(local, local_id, arena_x, arena_y, MULTIPLAYER_CELL_SIZE, RENDER_LAYER_SNAKE_BODY, RENDER_LAYER_SNAKE_HEAD) != 0)
      return 1;
  }
  else {
    if (renderer_submit_death_snake_grid_skin(local, multiplayer_session_get_player_skin(local_id), arena_x, arena_y, MULTIPLAYER_CELL_SIZE, multiplayer_session_get_player_death_animation_tick(local_id), multiplayer_session_get_death_animation_ticks(), RENDER_LAYER_SNAKE_BODY, RENDER_LAYER_SNAKE_HEAD) != 0)
      return 1;
  }

  if (renderer_submit_score_mode_minimap(opponent, hres) != 0)
    return 1;

  return renderer_draw_multiplayer_result_overlay(hres, vres);
}
