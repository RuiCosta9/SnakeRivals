/**
 * @file renderer.c
 * @brief Frame command queue, sorting and state render dispatcher.
 */

#include "renderer.h"
#include "renderer_internal.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assets.h"
#include "food.h"
#include "join_state.h"
#include "multiplayer_session.h"
#include "mouse_cursor.h"
#include "pause_state.h"
#include "powerup.h"
#include "singleplayer_state.h"
#include "snake.h"
#include "hud.h"
#include "sprite.h"
#include "state.h"
#include "text.h"
#include "ui_layout.h"
#include "vbe.h"
#include "video.h"
#include "game_over_state.h"

#define RENDERER_BUFFER_COUNT 3
#define RENDERER_MAX_COMMANDS 512
#define RENDERER_TEXT_MAX_LENGTH 96

typedef enum {
  RENDER_COMMAND_RECT,
  RENDER_COMMAND_SPRITE,
  RENDER_COMMAND_TEXT
} render_command_type_t;

typedef struct {
  render_command_type_t type;
  int16_t z;
  uint16_t order;

  union {
    struct {
      uint16_t x;
      uint16_t y;
      uint16_t width;
      uint16_t height;
      uint32_t color;
    } rect;

    struct {
      const sprite_t *sprite;
      uint16_t x;
      uint16_t y;
    } sprite;

    struct {
      char text[RENDERER_TEXT_MAX_LENGTH];
      uint16_t x;
      uint16_t y;
      uint8_t scale;
      uint32_t color;
    } text;
  } data;
} render_command_t;

static render_command_t commands[RENDERER_MAX_COMMANDS];
static size_t command_count = 0;
static uint16_t next_order = 0;

static unsigned int rendered_frames = 0;

static void renderer_reset_commands(void) {
  command_count = 0;
  next_order = 0;
}

static int renderer_push_command(render_command_t command) {
  if (command_count >= RENDERER_MAX_COMMANDS)
    return 1;

  command.order = next_order++;
  commands[command_count++] = command;

  return 0;
}

static int renderer_compare_commands(const void *lhs, const void *rhs) {
  const render_command_t *a = (const render_command_t *) lhs;
  const render_command_t *b = (const render_command_t *) rhs;

  if (a->z < b->z)
    return -1;

  if (a->z > b->z)
    return 1;

  if (a->order < b->order)
    return -1;

  if (a->order > b->order)
    return 1;

  return 0;
}

int renderer_init(void) {
  if (vbe_enable_triple_buffering() != 0)
    return 1;

  for (uint8_t i = 0; i < RENDERER_BUFFER_COUNT; i++) {
    if (renderer_clear(0x00) != 0)
      return 1;

    if (renderer_present() != 0)
      return 1;
  }

  rendered_frames = 0;
  renderer_reset_commands();

  return 0;
}

int renderer_shutdown(void) {
  renderer_reset_commands();
  return vbe_disable_triple_buffering();
}

int renderer_begin_frame(uint32_t clear_color) {
  renderer_reset_commands();
  return renderer_clear(clear_color);
}

int renderer_clear(uint32_t color) {
  return video_clear_screen(color);
}

int renderer_present(void) {
  int ret = vbe_present_draw_buffer();

  if (ret == 0)
    rendered_frames++;

  return ret;
}

int renderer_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  return video_draw_rectangle(x, y, width, height, color);
}

int renderer_draw_sprite(const sprite_t *sprite, uint16_t x, uint16_t y) {
  return sprite_draw(sprite, x, y);
}

int renderer_submit_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, int16_t z) {
  render_command_t command;

  command.type = RENDER_COMMAND_RECT;
  command.z = z;
  command.order = 0;

  command.data.rect.x = x;
  command.data.rect.y = y;
  command.data.rect.width = width;
  command.data.rect.height = height;
  command.data.rect.color = color;

  return renderer_push_command(command);
}

int renderer_submit_sprite(const sprite_t *sprite, uint16_t x, uint16_t y, int16_t z) {
  if (sprite == NULL)
    return 0;

  render_command_t command;

  command.type = RENDER_COMMAND_SPRITE;
  command.z = z;
  command.order = 0;

  command.data.sprite.sprite = sprite;
  command.data.sprite.x = x;
  command.data.sprite.y = y;

  return renderer_push_command(command);
}

int renderer_submit_text(const char *text, uint16_t x, uint16_t y, uint8_t scale, uint32_t color, int16_t z) {
  if (text == NULL)
    return 0;

  render_command_t command;

  command.type = RENDER_COMMAND_TEXT;
  command.z = z;
  command.order = 0;

  strncpy(command.data.text.text, text, RENDERER_TEXT_MAX_LENGTH - 1);
  command.data.text.text[RENDERER_TEXT_MAX_LENGTH - 1] = '\0';
  command.data.text.x = x;
  command.data.text.y = y;
  command.data.text.scale = scale;
  command.data.text.color = color;

  return renderer_push_command(command);
}

int renderer_flush(void) {
  qsort(commands, command_count, sizeof(render_command_t), renderer_compare_commands);

  for (size_t i = 0; i < command_count; i++) {
    int ret = 0;

    switch (commands[i].type) {
      case RENDER_COMMAND_RECT:
        ret = renderer_draw_rect(commands[i].data.rect.x, commands[i].data.rect.y, commands[i].data.rect.width, commands[i].data.rect.height, commands[i].data.rect.color);
        break;

      case RENDER_COMMAND_SPRITE:
        ret = renderer_draw_sprite(commands[i].data.sprite.sprite, commands[i].data.sprite.x, commands[i].data.sprite.y);
        break;

      case RENDER_COMMAND_TEXT:
        ret = text_draw(commands[i].data.text.text, commands[i].data.text.x, commands[i].data.text.y, commands[i].data.text.scale, commands[i].data.text.color);
        break;

      default:
        ret = 1;
        break;
    }

    if (ret != 0)
      return 1;
  }

  renderer_reset_commands();

  return renderer_present();
}

int renderer_render_state(app_state_t state) {
  int ret = 0;

  if (renderer_begin_frame(0x00) != 0)
    return 1;

  switch (state) {
    case STATE_MENU:
      ret = renderer_draw_menu();
      break;

    case STATE_PLAY_MENU:
      ret = renderer_draw_play_menu();
      break;

    case STATE_INSTRUCTIONS:
      ret = renderer_draw_instructions();
      break;

    case STATE_SINGLEPLAYER:
      ret = renderer_draw_singleplayer();
      break;

    case STATE_MULTIPLAYER_JOIN:
      ret = renderer_draw_join_menu();
      break;

    case STATE_MULTIPLAYER_HOST:
    case STATE_MULTIPLAYER_CLIENT:
      ret = renderer_draw_multiplayer();
      break;

    case STATE_PAUSE:
      ret = renderer_draw_pause();
      break;

    case STATE_GAME_OVER:
      ret = renderer_draw_game_over();
      break;

    case STATE_EXIT:
      ret = renderer_submit_rect(0, 0, vbe_get_hres(), vbe_get_vres(), 0x00, RENDER_LAYER_BACKGROUND);
      break;

    default:
      ret = renderer_submit_rect(0, 0, vbe_get_hres(), vbe_get_vres(), 0x00, RENDER_LAYER_BACKGROUND);
      break;
  }

  if (ret != 0)
    return 1;

  if (renderer_submit_cursor() != 0)
    return 1;

  return renderer_flush();
}

int renderer_render_current_state(void) {
  return renderer_render_state(state_get_current());
}
