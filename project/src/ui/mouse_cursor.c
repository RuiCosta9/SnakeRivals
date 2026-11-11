/**
 * @file mouse_cursor.c
 * @brief Fixed-point mouse cursor state and click-edge tracking.
 */

#include "mouse_cursor.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "vbe.h"

#define MOUSE_CURSOR_FP_SHIFT 21
#define MOUSE_CURSOR_FP_UNIT  (1 << MOUSE_CURSOR_FP_SHIFT)

#define MOUSE_CURSOR_SPEED_NUM 3
#define MOUSE_CURSOR_SPEED_DEN 2

static int32_t cursor_x_fp = 0;
static int32_t cursor_y_fp = 0;

static bool left_down = false;
static bool previous_left_down = false;
static bool left_clicked = false;
static bool moved = false;

static int32_t mouse_cursor_upper_bound_fp(uint16_t upper_bound) {
  if (upper_bound == 0)
    return 0;

  return ((int32_t) upper_bound - 1) * MOUSE_CURSOR_FP_UNIT;
}

static int32_t mouse_cursor_clamp_fp(int32_t value, uint16_t upper_bound) {
  int32_t upper_bound_fp = mouse_cursor_upper_bound_fp(upper_bound);

  if (value < 0)
    return 0;

  if (value > upper_bound_fp)
    return upper_bound_fp;

  return value;
}

static int32_t mouse_cursor_scale_delta(int16_t delta) {
  return ((int32_t) delta * MOUSE_CURSOR_SPEED_NUM * MOUSE_CURSOR_FP_UNIT) /
         MOUSE_CURSOR_SPEED_DEN;
}

static int16_t mouse_cursor_fp_to_screen(int32_t value_fp) {
  return (int16_t) ((value_fp + MOUSE_CURSOR_FP_UNIT / 2) / MOUSE_CURSOR_FP_UNIT);
}

void mouse_cursor_init(void) {
  cursor_x_fp = ((int32_t) vbe_get_hres() / 2) * MOUSE_CURSOR_FP_UNIT;
  cursor_y_fp = ((int32_t) vbe_get_vres() / 2) * MOUSE_CURSOR_FP_UNIT;

  left_down = false;
  previous_left_down = false;
  left_clicked = false;
  moved = false;
}

void mouse_cursor_update(const struct packet *packet) {
  if (packet == NULL) {
    left_clicked = false;
    moved = false;
    return;
  }

  previous_left_down = left_down;
  left_down = packet->lb;
  left_clicked = left_down && !previous_left_down;

  moved = false;

  int16_t delta_x = packet->delta_x;
  int16_t delta_y = packet->delta_y;

  if (packet->x_ov)
    delta_x = delta_x < 0 ? -255 : 255;

  if (packet->y_ov)
    delta_y = delta_y < 0 ? -255 : 255;

  int32_t previous_x_fp = cursor_x_fp;
  int32_t previous_y_fp = cursor_y_fp;

  cursor_x_fp = mouse_cursor_clamp_fp(cursor_x_fp + mouse_cursor_scale_delta(delta_x), vbe_get_hres());

  cursor_y_fp = mouse_cursor_clamp_fp(cursor_y_fp - mouse_cursor_scale_delta(delta_y), vbe_get_vres());

  moved = previous_x_fp != cursor_x_fp || previous_y_fp != cursor_y_fp;
}

int16_t mouse_cursor_get_x(void) {
  return mouse_cursor_fp_to_screen(cursor_x_fp);
}

int16_t mouse_cursor_get_y(void) {
  return mouse_cursor_fp_to_screen(cursor_y_fp);
}

bool mouse_cursor_was_left_clicked(void) {
  return left_clicked;
}

bool mouse_cursor_is_left_down(void) {
  return left_down;
}

bool mouse_cursor_has_moved(void) {
  return moved;
}
