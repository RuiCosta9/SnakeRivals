/**
 * @file renderer_game.c
 * @brief Singleplayer gameplay rendering.
 */

#include "renderer_internal.h"
#include "renderer.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "assets.h"
#include "food.h"
#include "hud.h"
#include "powerup.h"
#include "singleplayer_state.h"
#include "snake.h"
#include "text.h"
#include "vbe.h"

static uint16_t renderer_interpolate_u16(uint16_t start, uint16_t end, uint16_t step, uint16_t steps) {
  if (steps == 0 || step >= steps)
    return end;

  int16_t delta = (int16_t) end - (int16_t) start;
  return (uint16_t) ((int16_t) start + (delta * step) / steps);
}

static uint16_t renderer_ease_in_u16(uint16_t step, uint16_t steps) {
  if (steps == 0 || step >= steps)
    return steps;

  return (uint16_t) (((uint32_t) step * step) / steps);
}

static uint16_t renderer_add_offset_u16(uint16_t value, int16_t offset) {
  int16_t result = (int16_t) value + offset;
  return result < 0 ? 0 : (uint16_t) result;
}

static uint16_t renderer_interpolated_food_axis(uint16_t arena_origin, uint16_t cell_size, bool horizontal) {
  uint8_t count = singleplayer_state_get_food_animation_position_count();
  uint8_t step = singleplayer_state_get_animation_step();
  uint8_t steps = singleplayer_state_get_animation_steps();

  if (count <= 1 || steps == 0 || step >= steps) {
    position_t position = singleplayer_state_get_food_animation_position(count > 0 ? count - 1 : 0);
    return arena_origin + (uint16_t) (horizontal ? position.x : position.y) * cell_size;
  }

  uint8_t segment_count = count - 1;
  uint16_t scaled_step = (uint16_t) step * segment_count;
  uint8_t segment = (uint8_t) (scaled_step / steps);
  uint8_t segment_step = (uint8_t) (scaled_step % steps);

  if (segment >= segment_count) {
    position_t position = singleplayer_state_get_food_animation_position(count - 1);
    return arena_origin + (uint16_t) (horizontal ? position.x : position.y) * cell_size;
  }

  position_t start = singleplayer_state_get_food_animation_position(segment);
  position_t end = singleplayer_state_get_food_animation_position((uint8_t) (segment + 1));
  uint16_t start_pixel = arena_origin + (uint16_t) (horizontal ? start.x : start.y) * cell_size;
  uint16_t end_pixel = arena_origin + (uint16_t) (horizontal ? end.x : end.y) * cell_size;

  return renderer_interpolate_u16(start_pixel, end_pixel, segment_step, steps);
}

static bool renderer_death_fade_should_draw(uint16_t index, uint16_t tick, uint16_t step, uint16_t steps) {
  if (steps == 0)
    return false;

  if (step >= steps)
    return false;

  uint32_t fade = (uint32_t) step * step;
  uint32_t fade_limit = (uint32_t) steps * steps;
  uint16_t visibility = (uint16_t) (16 - (fade * 16) / fade_limit);
  uint16_t pattern = (uint16_t) ((index * 7 + tick * 5) & 0x0F);

  return pattern < visibility;
}

static int renderer_submit_singleplayer_snake(const snake_t *snake, uint16_t arena_x, uint16_t arena_y, uint16_t cell_size, int16_t body_layer, int16_t head_layer) {
  if (snake == NULL)
    return 0;

  const sprite_t *head = assets_get_snake_head_for_direction(renderer_get_snake_head_direction(snake));
  const sprite_t *body = assets_get_snake_body();
  const sprite_t *tail = assets_get_snake_tail_for_direction(renderer_get_snake_tail_direction(snake));
  uint16_t length = snake_get_length(snake);
  uint16_t previous_length = singleplayer_state_get_previous_snake_length();
  uint8_t step = singleplayer_state_get_animation_step();
  uint8_t steps = singleplayer_state_get_animation_steps();

  for (uint16_t i = length; i > 0; i--) {
    uint16_t index = i - 1;
    position_t segment = snake_get_segment(snake, index);
    position_t previous = index < previous_length ? singleplayer_state_get_previous_snake_segment(index) : segment;
    uint16_t current_x = arena_x + (uint16_t) segment.x * cell_size;
    uint16_t current_y = arena_y + (uint16_t) segment.y * cell_size;
    uint16_t previous_x = arena_x + (uint16_t) previous.x * cell_size;
    uint16_t previous_y = arena_y + (uint16_t) previous.y * cell_size;
    uint16_t x = renderer_interpolate_u16(previous_x, current_x, step, steps);
    uint16_t y = renderer_interpolate_u16(previous_y, current_y, step, steps);

    if (index == 0) {
      if (renderer_submit_sprite(head, x, y, head_layer) != 0)
        return 1;
    }
    else if (index == length - 1) {
      if (renderer_submit_sprite(tail, x, y, body_layer + index) != 0)
        return 1;
    }
    else {
      if (renderer_submit_sprite(body, x, y, body_layer + index) != 0)
        return 1;
    }
  }

  return 0;
}

static int renderer_submit_singleplayer_death_snake(const snake_t *snake, uint16_t arena_x, uint16_t arena_y, uint16_t cell_size, int16_t body_layer, int16_t head_layer) {
  if (snake == NULL)
    return 0;

  const sprite_t *head = assets_get_snake_head_for_direction(snake_get_current_direction(snake));
  const sprite_t *body = assets_get_snake_body();
  const sprite_t *tail = assets_get_snake_tail_for_direction(renderer_get_snake_tail_direction(snake));
  uint16_t length = snake_get_length(snake);
  uint16_t total_ticks = singleplayer_state_get_death_animation_ticks();
  uint16_t tick = singleplayer_state_get_death_animation_tick();
  uint16_t impact_ticks = SINGLEPLAYER_DEATH_IMPACT_TICKS;

  if (length == 0 || total_ticks == 0)
    return 0;

  if (tick < impact_ticks) {
    direction_t direction = snake_get_current_direction(snake);

    for (uint16_t i = length; i > 0; i--) {
      uint16_t index = i - 1;
      position_t segment = snake_get_segment(snake, index);
      uint16_t x = arena_x + (uint16_t) segment.x * cell_size;
      uint16_t y = arena_y + (uint16_t) segment.y * cell_size;
      uint16_t delayed_tick = tick > index * 2 ? tick - index * 2 : 0;
      uint16_t damping_tick = delayed_tick > impact_ticks ? impact_ticks : delayed_tick;
      int16_t amplitude = damping_tick < impact_ticks / 2 ? 1 : 0;
      int16_t shake = ((tick / 3 + index) % 2 == 0) ? amplitude : -amplitude;

      if (direction == DIRECTION_UP || direction == DIRECTION_DOWN)
        x = renderer_add_offset_u16(x, shake);
      else
        y = renderer_add_offset_u16(y, shake);

      if (index == 0) {
        if (renderer_submit_sprite(head, x, y, head_layer) != 0)
          return 1;
      }
      else if (index == length - 1) {
        if (renderer_submit_sprite(tail, x, y, body_layer + index) != 0)
          return 1;
      }
      else {
        if (renderer_submit_sprite(body, x, y, body_layer + index) != 0)
          return 1;
      }
    }

    return 0;
  }

  uint16_t collapse_ticks = total_ticks - impact_ticks;
  uint16_t collapse_tick = tick - impact_ticks;

  if (collapse_ticks == 0)
    return 0;

  for (uint16_t i = length; i > 0; i--) {
    uint16_t index = i - 1;
    position_t segment = snake_get_segment(snake, index);
    uint16_t delay = index * 2;
    uint16_t available_ticks = collapse_ticks > delay ? collapse_ticks - delay : 1;
    uint16_t segment_tick = collapse_tick > delay ? collapse_tick - delay : 0;
    uint16_t segment_progress = segment_tick >= available_ticks ? available_ticks : segment_tick;
    uint16_t eased_progress = renderer_ease_in_u16(segment_progress, available_ticks);
    uint16_t x = arena_x + (uint16_t) segment.x * cell_size;
    uint16_t y = arena_y + (uint16_t) segment.y * cell_size;

    if (!renderer_death_fade_should_draw(index, tick, eased_progress, available_ticks))
      continue;

    if (index == 0) {
      if (renderer_submit_sprite(head, x, y, head_layer) != 0)
        return 1;
    }
    else if (index == length - 1) {
      if (renderer_submit_sprite(tail, x, y, body_layer + index) != 0)
        return 1;
    }
    else {
      if (renderer_submit_sprite(body, x, y, body_layer + index) != 0)
        return 1;
    }
  }

  return 0;
}

int renderer_draw_singleplayer(void) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  const snake_t *snake = singleplayer_state_get_snake();
  const food_t *food = singleplayer_state_get_food();
  const powerup_t *powerup = singleplayer_state_get_powerup();

  uint16_t arena_w = SINGLEPLAYER_GRID_COLUMNS * SINGLEPLAYER_CELL_SIZE;
  uint16_t arena_h = SINGLEPLAYER_GRID_ROWS * SINGLEPLAYER_CELL_SIZE;
  uint16_t arena_x = hres > arena_w ? (hres - arena_w) / 2 : 0;
  uint16_t arena_y = vres > arena_h ? (vres - arena_h) / 2 : 0;
  uint16_t border = SINGLEPLAYER_CELL_SIZE;

  if (renderer_submit_rect(0, 0, hres, vres, 0x00, RENDER_LAYER_BACKGROUND) != 0)
    return 1;

  if (renderer_submit_play_background(arena_x, arena_y, arena_w, arena_h, 0, 0) != 0)
    return 1;

  if (arena_x >= border) {
    if (renderer_submit_rect(arena_x - border, arena_y, border, arena_h, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if ((uint32_t) arena_x + arena_w + border <= hres) {
    if (renderer_submit_rect(arena_x + arena_w, arena_y, border, arena_h, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if (arena_y >= border) {
    if (renderer_submit_rect(arena_x >= border ? arena_x - border : arena_x, arena_y - border, arena_x >= border && (uint32_t) arena_x + arena_w + border <= hres ? arena_w + 2 * border : arena_w, border, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if ((uint32_t) arena_y + arena_h + border <= vres) {
    if (renderer_submit_rect(arena_x >= border ? arena_x - border : arena_x, arena_y + arena_h, arena_x >= border && (uint32_t) arena_x + arena_w + border <= hres ? arena_w + 2 * border : arena_w, border, 0x08, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if (hud_draw_gameplay(singleplayer_state_get_score(), snake, arena_x, arena_y >= 32 ? arena_y - 32 : 8) != 0)
    return 1;

  {
    uint16_t inventory_slot = 50;
    uint16_t inventory_gap = 14;
    uint16_t inventory_w = (uint16_t) (inventory_slot * 2 + inventory_gap);
    uint16_t inventory_x = arena_x + arena_w > inventory_w ? (uint16_t) (arena_x + arena_w - inventory_w) : arena_x;
    uint16_t inventory_y = arena_y > 74 ? (uint16_t) (arena_y - 74) : 8;
    bool next = singleplayer_state_has_queued_powerup();
    powerup_type_t next_type = next ? singleplayer_state_get_queued_powerup() : POWERUP_DOUBLE_POINTS;
    uint16_t status_x = (uint16_t) (inventory_x + inventory_w + 12);
    uint16_t status_y = (uint16_t) (inventory_y + 4);
    uint8_t effect_scale = 2;

    if ((uint32_t) status_x + 150 > hres && hres > inventory_w + 170) {
      inventory_x = (uint16_t) (hres - inventory_w - 150);
      status_x = (uint16_t) (inventory_x + inventory_w + 12);
    }

    if (renderer_submit_powerup_inventory_slot(inventory_x, inventory_y, inventory_slot, "ITEM", singleplayer_state_has_held_powerup(), singleplayer_state_get_held_powerup(), RENDER_LAYER_UI + 5) != 0)
      return 1;

    if (renderer_submit_powerup_inventory_slot((uint16_t) (inventory_x + inventory_slot + inventory_gap), inventory_y, inventory_slot, "NEXT", next, next_type, RENDER_LAYER_UI + 5) != 0)
      return 1;

    if (singleplayer_state_has_shield()) {
      if (renderer_submit_text("SHIELD", status_x, status_y, effect_scale, 0x0A, RENDER_LAYER_UI + 8) != 0)
        return 1;
      status_y = (uint16_t) (status_y + TEXT_FONT_HEIGHT * effect_scale + 4);
    }

    uint8_t effect_count = singleplayer_state_get_effect_count();
    for (uint8_t i = 0; i < effect_count && i < SINGLEPLAYER_EFFECT_SLOTS; i++) {
      char effect_text[24];
      uint16_t ticks = singleplayer_state_get_effect_ticks(i);
      uint16_t seconds = (ticks + 59) / 60;

      snprintf(effect_text, sizeof(effect_text), "%s %u", renderer_powerup_label(singleplayer_state_get_effect_type(i)), (unsigned) seconds);

      if (renderer_submit_text(effect_text, status_x, status_y, effect_scale, 0x0E, RENDER_LAYER_UI + 8) != 0)
        return 1;

      status_y = (uint16_t) (status_y + TEXT_FONT_HEIGHT * effect_scale + 4);
    }
  }

  if (food_is_active(food)) {
    uint16_t food_x = renderer_interpolated_food_axis(arena_x, SINGLEPLAYER_CELL_SIZE, true);
    uint16_t food_y = renderer_interpolated_food_axis(arena_y, SINGLEPLAYER_CELL_SIZE, false);

    if (singleplayer_state_is_bonus_food_active()) {
      if (renderer_submit_rect(food_x, food_y, SINGLEPLAYER_CELL_SIZE, SINGLEPLAYER_CELL_SIZE, 0x0E, RENDER_LAYER_FOOD) != 0)
        return 1;
    }
    else {
      const sprite_t *food_sprite = assets_get_food();

      if (renderer_submit_sprite(food_sprite, food_x, food_y, RENDER_LAYER_FOOD) != 0)
        return 1;
    }
  }

  {
    const food_field_t *frenzy = singleplayer_state_get_frenzy_field();

    if (frenzy != NULL) {
      uint8_t frenzy_count = food_field_get_count(frenzy);

      for (uint8_t i = 0; i < frenzy_count; i++) {
        if (!food_field_item_active(frenzy, i))
          continue;

        position_t frenzy_position = food_field_item_position(frenzy, i);
        uint16_t frenzy_x = arena_x + (uint16_t) frenzy_position.x * SINGLEPLAYER_CELL_SIZE;
        uint16_t frenzy_y = arena_y + (uint16_t) frenzy_position.y * SINGLEPLAYER_CELL_SIZE;
        const sprite_t *food_sprite = assets_get_frenzy_food();

        if (renderer_submit_sprite(food_sprite, frenzy_x, frenzy_y, RENDER_LAYER_FOOD) != 0)
          return 1;
      }
    }
  }

  if (powerup_is_active(powerup)) {
    position_t powerup_position = powerup_get_position(powerup);
    uint16_t powerup_x = arena_x + (uint16_t) powerup_position.x * SINGLEPLAYER_CELL_SIZE;
    uint16_t powerup_y = arena_y + (uint16_t) powerup_position.y * SINGLEPLAYER_CELL_SIZE;

    if (renderer_submit_powerup_icon(powerup_get_type(powerup), powerup_x, powerup_y, SINGLEPLAYER_CELL_SIZE, RENDER_LAYER_POWER_UP) != 0)
      return 1;
  }

  if (snake == NULL)
    return 0;

  if (singleplayer_state_is_death_animating()) {
    if (renderer_submit_singleplayer_death_snake(snake, arena_x, arena_y, SINGLEPLAYER_CELL_SIZE, RENDER_LAYER_SNAKE_BODY, RENDER_LAYER_SNAKE_HEAD) != 0)
      return 1;
  }
  else {
    if (renderer_submit_singleplayer_snake(snake, arena_x, arena_y, SINGLEPLAYER_CELL_SIZE, RENDER_LAYER_SNAKE_BODY, RENDER_LAYER_SNAKE_HEAD) != 0)
      return 1;
  }

  if (!snake_is_alive(snake)) {
    if (renderer_submit_text("GAME OVER", hres / 2 - text_width("GAME OVER", 5) / 2, vres / 2 - 40, 5, 0x0F, RENDER_LAYER_UI) != 0)
      return 1;
  }

  return 0;
}
