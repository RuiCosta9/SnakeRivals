/**
 * @file renderer_helpers.c
 * @brief Shared renderer helpers for text, backgrounds, snakes and UI widgets.
 */

#include "renderer_internal.h"
#include "renderer.h"

#include <stddef.h>
#include <stdint.h>

#include "assets.h"
#include "mouse_cursor.h"
#include "powerup.h"
#include "snake.h"
#include "sprite.h"
#include "text.h"
#include "ui_layout.h"
#include "vbe.h"

int renderer_submit_cursor(void) {
  const sprite_t *cursor = assets_get_cursor();

  if (cursor == NULL)
    return 0;

  int16_t cursor_x = mouse_cursor_get_x();
  int16_t cursor_y = mouse_cursor_get_y();

  uint16_t x = cursor_x < 0 ? 0 : (uint16_t) cursor_x;
  uint16_t y = cursor_y < 0 ? 0 : (uint16_t) cursor_y;

  uint16_t width = sprite_get_width(cursor);
  uint16_t height = sprite_get_height(cursor);
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  if (width > 0 && (uint32_t) x + width > hres)
    x = hres > width ? hres - width : 0;

  if (height > 0 && (uint32_t) y + height > vres)
    y = vres > height ? vres - height : 0;

  return renderer_submit_sprite(cursor, x, y, RENDER_LAYER_CURSOR);
}

#define RENDERER_BG_FIELD_COLOR   0x01  /* azul escuro (campo) */
#define RENDERER_BG_CHECKER_COLOR 0x09  /* azul claro (xadrez) */
#define RENDERER_BG_CHECKER_PX    64
#define RENDERER_BG_MAX_SQUARES   200

int renderer_submit_play_background(uint16_t area_x, uint16_t area_y, uint16_t area_w, uint16_t area_h, int32_t scroll_x, int32_t scroll_y) {
  if (area_w == 0 || area_h == 0)
    return 0;

  if (renderer_submit_rect(area_x, area_y, area_w, area_h, RENDERER_BG_FIELD_COLOR, RENDER_LAYER_BACKGROUND) != 0)
    return 1;

  const int32_t square = RENDERER_BG_CHECKER_PX;
  int32_t world_left = scroll_x < 0 ? 0 : scroll_x;
  int32_t world_top = scroll_y < 0 ? 0 : scroll_y;
  int32_t world_right = world_left + area_w;
  int32_t world_bottom = world_top + area_h;
  int32_t first_col = world_left / square;
  int32_t first_row = world_top / square;
  uint16_t drawn = 0;

  for (int32_t col = first_col; col * square < world_right && drawn < RENDERER_BG_MAX_SQUARES; col++) {
    for (int32_t row = first_row; row * square < world_bottom && drawn < RENDERER_BG_MAX_SQUARES; row++) {
      if (((col + row) & 1) == 0)
        continue;

      int32_t sx0 = col * square;
      int32_t sy0 = row * square;
      int32_t wx0 = sx0 < world_left ? world_left : sx0;
      int32_t wy0 = sy0 < world_top ? world_top : sy0;
      int32_t wx1 = sx0 + square > world_right ? world_right : sx0 + square;
      int32_t wy1 = sy0 + square > world_bottom ? world_bottom : sy0 + square;

      if (wx1 <= wx0 || wy1 <= wy0)
        continue;

      uint16_t screen_x = area_x + (uint16_t) (wx0 - world_left);
      uint16_t screen_y = area_y + (uint16_t) (wy0 - world_top);

      if (renderer_submit_rect(screen_x, screen_y, (uint16_t) (wx1 - wx0), (uint16_t) (wy1 - wy0), RENDERER_BG_CHECKER_COLOR, RENDER_LAYER_BACKGROUND + 1) != 0)
        return 1;

      drawn++;
    }
  }

  return 0;
}

int renderer_submit_button_visual_state(const ui_rect_t *rect, const char *label, bool selected, int16_t z) {
  if (rect == NULL || label == NULL)
    return 1;

  uint16_t shadow_offset = rect->height / 12;

  if (shadow_offset == 0)
    shadow_offset = 1;

  if (selected) {
    uint16_t outline = 4;
    uint16_t ox = rect->x > outline ? rect->x - outline : 0;
    uint16_t oy = rect->y > outline ? rect->y - outline : 0;

    if (renderer_submit_rect(ox, oy, rect->width + 2 * outline, rect->height + 2 * outline, 0x0E, z) != 0)
      return 1;
  }

  if (renderer_submit_rect(rect->x + shadow_offset, rect->y + shadow_offset, rect->width, rect->height, 0x00, z + 1) != 0)
    return 1;

  if (renderer_submit_rect(rect->x, rect->y, rect->width, rect->height, selected ? 0x0B : 0x08, z + 2) != 0)
    return 1;

  if (rect->width > 8 && rect->height > 8) {
    if (renderer_submit_rect(rect->x + 4, rect->y + 4, rect->width - 8, rect->height - 8, selected ? 0x01 : 0x00, z + 3) != 0)
      return 1;
  }

  uint8_t scale = rect->height / 10;

  if (scale < 2)
    scale = 2;

  if (scale > 6)
    scale = 6;

  while (scale > 1 && text_width(label, scale) > rect->width - rect->width / 8)
    scale--;

  uint16_t label_width = text_width(label, scale);
  uint16_t label_height = TEXT_FONT_HEIGHT * scale;
  uint16_t label_x = rect->x;
  uint16_t label_y = rect->y;

  if (rect->width > label_width)
    label_x = rect->x + (rect->width - label_width) / 2;

  if (rect->height > label_height)
    label_y = rect->y + (rect->height - label_height) / 2;

  return renderer_submit_text(label, label_x, label_y, scale, selected ? 0x0F : 0x0E, z + 4);
}

int renderer_submit_button_visual(const ui_rect_t *rect, const char *label, int16_t z) {
  return renderer_submit_button_visual_state(rect, label, false, z);
}

int renderer_submit_button_icon(const sprite_t *sprite, const ui_rect_t *rect, int16_t z) {
  if (sprite == NULL || rect == NULL)
    return 0;

  uint16_t sprite_width = sprite_get_width(sprite);
  uint16_t sprite_height = sprite_get_height(sprite);
  uint16_t icon_x = rect->x + rect->height / 2;
  uint16_t icon_y = rect->y;

  if (rect->height > sprite_height)
    icon_y = rect->y + (rect->height - sprite_height) / 2;

  if (rect->width > rect->height + sprite_width)
    icon_x = rect->x + rect->height / 2 - sprite_width / 2;

  return renderer_submit_sprite(sprite, icon_x, icon_y, z);
}

int renderer_submit_back_arrow(uint16_t x, uint16_t y, uint16_t size, int16_t z) {
  if (renderer_submit_rect((uint16_t) (x + 3), (uint16_t) (y + 3), size, size, 0x00, z) != 0)
    return 1;

  if (renderer_submit_rect(x, y, size, size, 0x08, z + 1) != 0)
    return 1;

  if (size > 8) {
    if (renderer_submit_rect((uint16_t) (x + 4), (uint16_t) (y + 4), (uint16_t) (size - 8), (uint16_t) (size - 8), 0x01, z + 2) != 0)
      return 1;
  }

  const sprite_t *arrow = assets_get_back_arrow();

  if (arrow != NULL) {
    uint16_t arrow_w = sprite_get_width(arrow);
    uint16_t arrow_h = sprite_get_height(arrow);
    uint16_t arrow_x = x + (size > arrow_w ? (size - arrow_w) / 2 : 0);
    uint16_t arrow_y = y + (size > arrow_h ? (size - arrow_h) / 2 : 0);
    return renderer_submit_sprite(arrow, arrow_x, arrow_y, z + 4);
  }

  uint16_t unit = size / 8;

  if (unit < 3)
    unit = 3;

  uint16_t center_y = (uint16_t) (y + size / 2);

  if (renderer_submit_rect((uint16_t) (x + 3 * unit), (uint16_t) (center_y - unit / 2), (uint16_t) (size - 5 * unit), unit, 0x0F, z + 4) != 0)
    return 1;

  for (uint8_t row = 0; row < 5; row++) {
    uint8_t distance = row > 2 ? (uint8_t) (row - 2) : (uint8_t) (2 - row);
    uint16_t width = (uint16_t) (3 - distance) * unit;
    uint16_t row_y = (uint16_t) (center_y + ((int16_t) row - 2) * (int16_t) unit - (int16_t) unit / 2);

    if (renderer_submit_rect((uint16_t) (x + unit), row_y, width, unit, 0x0F, z + 4) != 0)
      return 1;
  }

  return 0;
}


int renderer_submit_centered_text(const char *text, uint16_t center_x, uint16_t y, uint8_t scale, uint32_t color, int16_t z) {
  uint16_t width = text_width(text, scale);
  uint16_t x = center_x > width / 2 ? center_x - width / 2 : 0;

  return renderer_submit_text(text, x, y, scale, color, z);
}

int renderer_submit_arena_border(uint16_t arena_x, uint16_t arena_y, uint16_t arena_w, uint16_t arena_h, uint16_t border, uint32_t color) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  if (arena_x >= border) {
    if (renderer_submit_rect(arena_x - border, arena_y, border, arena_h, color, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if ((uint32_t) arena_x + arena_w + border <= hres) {
    if (renderer_submit_rect(arena_x + arena_w, arena_y, border, arena_h, color, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if (arena_y >= border) {
    uint16_t x = arena_x >= border ? arena_x - border : arena_x;
    uint16_t w = arena_x >= border && (uint32_t) arena_x + arena_w + border <= hres ? arena_w + 2 * border : arena_w;

    if (renderer_submit_rect(x, arena_y - border, w, border, color, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  if ((uint32_t) arena_y + arena_h + border <= vres) {
    uint16_t x = arena_x >= border ? arena_x - border : arena_x;
    uint16_t w = arena_x >= border && (uint32_t) arena_x + arena_w + border <= hres ? arena_w + 2 * border : arena_w;

    if (renderer_submit_rect(x, arena_y + arena_h, w, border, color, RENDER_LAYER_WORLD) != 0)
      return 1;
  }

  return 0;
}

int renderer_submit_snake_cells(const snake_t *snake, uint16_t arena_x, uint16_t arena_y, uint16_t cell_size, uint32_t body_color, uint32_t head_color, int16_t z) {
  if (snake == NULL || cell_size == 0)
    return 0;

  uint16_t length = snake_get_length(snake);

  for (uint16_t i = length; i > 0; i--) {
    uint16_t index = i - 1;
    position_t segment = snake_get_segment(snake, index);
    uint16_t x = arena_x + (uint16_t) segment.x * cell_size;
    uint16_t y = arena_y + (uint16_t) segment.y * cell_size;
    uint32_t color = index == 0 ? head_color : body_color;

    if (renderer_submit_rect(x, y, cell_size, cell_size, color, z + (int16_t) index) != 0)
      return 1;
  }

  return 0;
}

int renderer_submit_snake(const snake_t *snake, uint16_t arena_x, uint16_t arena_y, uint16_t cell_size, int16_t body_layer, int16_t head_layer) {
  if (snake == NULL)
    return 0;

  const sprite_t *head = assets_get_snake_head_for_direction(renderer_get_snake_head_direction(snake));
  const sprite_t *body = assets_get_snake_body();
  const sprite_t *tail = assets_get_snake_tail_for_direction(renderer_get_snake_tail_direction(snake));
  uint16_t length = snake_get_length(snake);

  for (uint16_t i = length; i > 0; i--) {
    uint16_t index = i - 1;
    position_t segment = snake_get_segment(snake, index);
    uint16_t x = arena_x + (uint16_t) segment.x * cell_size;
    uint16_t y = arena_y + (uint16_t) segment.y * cell_size;

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


static uint16_t renderer_death_ease_in_u16(uint16_t step, uint16_t steps) {
  if (steps == 0 || step >= steps)
    return steps;

  return (uint16_t) (((uint32_t) step * step) / steps);
}

static uint16_t renderer_death_add_offset_u16(uint16_t value, int16_t offset) {
  int16_t result = (int16_t) value + offset;
  return result < 0 ? 0 : (uint16_t) result;
}

static bool renderer_death_fade_should_draw(uint16_t index, uint16_t tick, uint16_t step, uint16_t steps) {
  if (steps == 0 || step >= steps)
    return false;

  uint32_t fade = (uint32_t) step * step;
  uint32_t fade_limit = (uint32_t) steps * steps;
  uint16_t visibility = (uint16_t) (16 - (fade * 16) / fade_limit);
  uint16_t pattern = (uint16_t) ((index * 7 + tick * 5) & 0x0F);

  return pattern < visibility;
}

static int renderer_submit_death_snake_sprite(const sprite_t *sprite, uint16_t x, uint16_t y, int16_t layer) {
  if (sprite == NULL)
    return 0;

  return renderer_submit_sprite(sprite, x, y, layer);
}

int renderer_submit_death_snake_grid_skin(const snake_t *snake, uint8_t skin, uint16_t arena_x, uint16_t arena_y, uint16_t cell_size, uint16_t tick, uint16_t total_ticks, int16_t body_layer, int16_t head_layer) {
  if (snake == NULL || total_ticks == 0)
    return 0;

  const sprite_t *head = assets_get_snake_head_for_skin_direction(skin, snake_get_current_direction(snake));
  const sprite_t *body = assets_get_snake_body_for_skin(skin);
  const sprite_t *tail = assets_get_snake_tail_for_skin_direction(skin, renderer_get_snake_tail_direction(snake));
  uint16_t length = snake_get_length(snake);
  uint16_t impact_ticks = MULTIPLAYER_DEATH_IMPACT_TICKS;

  if (length == 0 || tick >= total_ticks)
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
      const sprite_t *sprite = index == 0 ? head : (index == length - 1 ? tail : body);
      int16_t layer = index == 0 ? head_layer : body_layer + (int16_t) index;

      if (direction == DIRECTION_UP || direction == DIRECTION_DOWN)
        x = renderer_death_add_offset_u16(x, shake);
      else
        y = renderer_death_add_offset_u16(y, shake);

      if (renderer_submit_death_snake_sprite(sprite, x, y, layer) != 0)
        return 1;
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
    uint16_t eased_progress = renderer_death_ease_in_u16(segment_progress, available_ticks);
    uint16_t x = arena_x + (uint16_t) segment.x * cell_size;
    uint16_t y = arena_y + (uint16_t) segment.y * cell_size;
    const sprite_t *sprite = index == 0 ? head : (index == length - 1 ? tail : body);
    int16_t layer = index == 0 ? head_layer : body_layer + (int16_t) index;

    if (!renderer_death_fade_should_draw(index, tick, eased_progress, available_ticks))
      continue;

    if (renderer_submit_death_snake_sprite(sprite, x, y, layer) != 0)
      return 1;
  }

  return 0;
}

int renderer_submit_death_snake_grid(const snake_t *snake, uint16_t arena_x, uint16_t arena_y, uint16_t cell_size, uint16_t tick, uint16_t total_ticks, int16_t body_layer, int16_t head_layer) {
  return renderer_submit_death_snake_grid_skin(snake, ASSETS_SNAKE_SKIN_GREEN, arena_x, arena_y, cell_size, tick, total_ticks, body_layer, head_layer);
}

int renderer_submit_death_snake_camera_skin(const snake_t *snake, uint8_t skin, int32_t camera_x, int32_t camera_y, uint16_t cell_size, uint16_t y_offset, uint16_t viewport_h, uint16_t tick, uint16_t total_ticks, int16_t body_layer, int16_t head_layer) {
  if (snake == NULL || total_ticks == 0)
    return 0;

  const sprite_t *head = assets_get_snake_head_for_skin_direction(skin, snake_get_current_direction(snake));
  const sprite_t *body = assets_get_snake_body_for_skin(skin);
  const sprite_t *tail = assets_get_snake_tail_for_skin_direction(skin, renderer_get_snake_tail_direction(snake));
  uint16_t length = snake_get_length(snake);
  uint16_t impact_ticks = MULTIPLAYER_DEATH_IMPACT_TICKS;
  uint16_t hres = vbe_get_hres();

  if (length == 0 || tick >= total_ticks)
    return 0;

  for (uint16_t i = length; i > 0; i--) {
    uint16_t index = i - 1;
    position_t segment = snake_get_segment(snake, index);
    int32_t screen_x = (int32_t) segment.x * cell_size - camera_x;
    int32_t screen_y = (int32_t) segment.y * cell_size - camera_y + y_offset;
    const sprite_t *sprite = index == 0 ? head : (index == length - 1 ? tail : body);
    int16_t layer = index == 0 ? head_layer : body_layer + (int16_t) index;

    if (tick < impact_ticks) {
      direction_t direction = snake_get_current_direction(snake);
      uint16_t delayed_tick = tick > index * 2 ? tick - index * 2 : 0;
      uint16_t damping_tick = delayed_tick > impact_ticks ? impact_ticks : delayed_tick;
      int16_t amplitude = damping_tick < impact_ticks / 2 ? 1 : 0;
      int16_t shake = ((tick / 3 + index) % 2 == 0) ? amplitude : -amplitude;

      if (direction == DIRECTION_UP || direction == DIRECTION_DOWN)
        screen_x += shake;
      else
        screen_y += shake;
    }
    else {
      uint16_t collapse_ticks = total_ticks - impact_ticks;
      uint16_t collapse_tick = tick - impact_ticks;
      uint16_t delay = index * 2;
      uint16_t available_ticks = collapse_ticks > delay ? collapse_ticks - delay : 1;
      uint16_t segment_tick = collapse_tick > delay ? collapse_tick - delay : 0;
      uint16_t segment_progress = segment_tick >= available_ticks ? available_ticks : segment_tick;
      uint16_t eased_progress = renderer_death_ease_in_u16(segment_progress, available_ticks);

      if (!renderer_death_fade_should_draw(index, tick, eased_progress, available_ticks))
        continue;
    }

    if (screen_x < 0 || screen_y < (int32_t) y_offset)
      continue;

    if (screen_x + cell_size > hres || screen_y + cell_size > y_offset + viewport_h)
      continue;

    if (renderer_submit_death_snake_sprite(sprite, (uint16_t) screen_x, (uint16_t) screen_y, layer) != 0)
      return 1;
  }

  return 0;
}

int renderer_submit_death_snake_camera(const snake_t *snake, int32_t camera_x, int32_t camera_y, uint16_t cell_size, uint16_t y_offset, uint16_t viewport_h, uint16_t tick, uint16_t total_ticks, int16_t body_layer, int16_t head_layer) {
  return renderer_submit_death_snake_camera_skin(snake, ASSETS_SNAKE_SKIN_GREEN, camera_x, camera_y, cell_size, y_offset, viewport_h, tick, total_ticks, body_layer, head_layer);
}

uint32_t renderer_powerup_color(powerup_type_t type) {
  switch (type) {
    case POWERUP_DOUBLE_POINTS:
      return 0x0E;

    case POWERUP_GHOST:
      return 0x0D;

    case POWERUP_SHIELD:
      return 0x0A;

    case POWERUP_MAGNET:
      return 0x0B;

    case POWERUP_FRENZY:
      return 0x0C;

    case POWERUP_BONUS_FOOD:
      return 0x06;

    case POWERUP_SPEED_BOOST:
      return 0x0A;

    case POWERUP_SLOW_ENEMY:
      return 0x09;

    case POWERUP_FORCE_GROWTH:
      return 0x05;

    default:
      return 0x0F;
  }
}

const char *renderer_powerup_label(powerup_type_t type) {
  switch (type) {
    case POWERUP_DOUBLE_POINTS:
      return "2X";

    case POWERUP_GHOST:
      return "GHOST";

    case POWERUP_MAGNET:
      return "MAGNET";

    case POWERUP_SHIELD:
      return "SHIELD";

    case POWERUP_FRENZY:
      return "FRENZY";

    case POWERUP_BONUS_FOOD:
      return "BONUS";

    case POWERUP_SPEED_BOOST:
      return "SPEED";

    case POWERUP_SLOW_ENEMY:
      return "SLOW";

    case POWERUP_FORCE_GROWTH:
      return "GROW";

    default:
      return "";
  }
}

int renderer_submit_powerup_icon(powerup_type_t type, uint16_t x, uint16_t y, uint16_t cell_size, int16_t z) {
  const sprite_t *sprite = assets_get_power_up_for_type(type);
  uint32_t color = renderer_powerup_color(type);
  const char *label = renderer_powerup_label(type);

  if (sprite != NULL && sprite_get_width(sprite) <= cell_size && sprite_get_height(sprite) <= cell_size) {
    uint16_t icon_x = x + (cell_size - sprite_get_width(sprite)) / 2;
    uint16_t icon_y = y + (cell_size - sprite_get_height(sprite)) / 2;
    return renderer_submit_sprite(sprite, icon_x, icon_y, z);
  }

  if (renderer_submit_rect(x, y, cell_size, cell_size, color, z) != 0)
    return 1;

  if (cell_size >= 22 && label != NULL && label[0] != '\0') {
    char short_label[3];
    uint8_t scale = 1;
    uint16_t label_width;
    uint16_t label_x;
    uint16_t label_y;

    short_label[0] = label[0];
    short_label[1] = label[1] != '\0' ? label[1] : ' ';
    short_label[2] = '\0';

    label_width = text_width(short_label, scale);
    label_x = x + (cell_size > label_width ? (cell_size - label_width) / 2 : 0);
    label_y = y + (cell_size > TEXT_FONT_HEIGHT * scale ? (cell_size - TEXT_FONT_HEIGHT * scale) / 2 : 0);

    if (renderer_submit_text(short_label, label_x, label_y, scale, 0x0F, z + 1) != 0)
      return 1;
  }

  return 0;
}

direction_t renderer_get_snake_head_direction(const snake_t *snake) {
  if (snake == NULL)
    return DIRECTION_RIGHT;

  return snake_get_next_direction(snake);
}

direction_t renderer_get_snake_tail_direction(const snake_t *snake) {
  if (snake == NULL)
    return DIRECTION_RIGHT;

  uint16_t length = snake_get_length(snake);

  if (length < 2)
    return snake_get_current_direction(snake);

  for (uint16_t i = length - 1; i > 0; i--) {
    position_t tail = snake_get_segment(snake, i);
    position_t previous = snake_get_segment(snake, i - 1);

    if (previous.x > tail.x)
      return DIRECTION_RIGHT;

    if (previous.x < tail.x)
      return DIRECTION_LEFT;

    if (previous.y > tail.y)
      return DIRECTION_DOWN;

    if (previous.y < tail.y)
      return DIRECTION_UP;
  }

  return snake_get_current_direction(snake);
}
