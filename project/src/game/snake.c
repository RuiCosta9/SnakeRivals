/**
 * @file snake.c
 * @brief Snake movement, growth and body-management implementation.
 */

#include "snake.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static bool snake_direction_is_opposite(direction_t first, direction_t second) {
  return (first == DIRECTION_UP && second == DIRECTION_DOWN) || (first == DIRECTION_DOWN && second == DIRECTION_UP) || (first == DIRECTION_LEFT && second == DIRECTION_RIGHT) || (first == DIRECTION_RIGHT && second == DIRECTION_LEFT);
}

static bool snake_direction_is_valid(direction_t direction) {
  return direction >= DIRECTION_UP && direction <= DIRECTION_LEFT;
}

static position_t snake_next_head_position(position_t head, direction_t direction) {
  switch (direction) {
    case DIRECTION_UP:
      head.y--;
      break;

    case DIRECTION_RIGHT:
      head.x++;
      break;

    case DIRECTION_DOWN:
      head.y++;
      break;

    case DIRECTION_LEFT:
      head.x--;
      break;

    default:
      break;
  }

  return head;
}

static bool snake_position_hits_body_without_tail(const snake_t *snake, position_t position) {
  if (snake == NULL)
    return false;

  if (snake->length <= 2)
    return false;

  for (uint16_t i = 1; i + 1 < snake->length; i++) {
    if (snake->body[i].x == position.x && snake->body[i].y == position.y)
      return true;
  }

  return false;
}

void snake_init(snake_t *snake, uint16_t grid_width, uint16_t grid_height) {
  if (snake == NULL)
    return;

  if (grid_width < SNAKE_INITIAL_LENGTH + 2)
    grid_width = SNAKE_INITIAL_LENGTH + 2;

  if (grid_height < 3)
    grid_height = 3;

  snake->length = SNAKE_INITIAL_LENGTH;
  snake->grid_width = grid_width;
  snake->grid_height = grid_height;
  snake->current_direction = DIRECTION_RIGHT;
  snake->next_direction = DIRECTION_RIGHT;
  snake->alive = true;

  int16_t head_x = (int16_t) (grid_width / 2);
  int16_t head_y = (int16_t) (grid_height / 2);

  for (uint16_t i = 0; i < snake->length; i++) {
    snake->body[i].x = head_x - (int16_t) i;
    snake->body[i].y = head_y;
  }
}

int snake_set_body(snake_t *snake, const position_t *segments, uint16_t length, direction_t direction) {
  if (snake == NULL || segments == NULL)
    return 1;

  if (length == 0 || length > SNAKE_MAX_SEGMENTS)
    return 1;

  if (!snake_direction_is_valid(direction))
    return 1;

  for (uint16_t i = 0; i < length; i++) {
    if (segments[i].x < 0 || segments[i].y < 0)
      return 1;

    if (segments[i].x >= (int16_t) snake->grid_width || segments[i].y >= (int16_t) snake->grid_height)
      return 1;
  }

  snake->length = length;
  snake->current_direction = direction;
  snake->next_direction = direction;
  snake->alive = true;

  for (uint16_t i = 0; i < length; i++)
    snake->body[i] = segments[i];

  return 0;
}

void snake_set_next_direction(snake_t *snake, direction_t direction) {
  if (snake == NULL)
    return;

  if (!snake->alive)
    return;

  if (!snake_direction_is_valid(direction))
    return;

  if (snake_direction_is_opposite(snake->current_direction, direction))
    return;

  snake->next_direction = direction;
}

void snake_set_alive(snake_t *snake, bool alive) {
  if (snake == NULL)
    return;

  snake->alive = alive;
}

void snake_move(snake_t *snake) {
  if (snake == NULL)
    return;

  if (!snake->alive)
    return;

  if (snake->length == 0)
    return;

  snake->current_direction = snake->next_direction;

  position_t new_head = snake_next_head_position(snake->body[0], snake->current_direction);

  if (new_head.x < 0 || new_head.y < 0 ||
      new_head.x >= (int16_t) snake->grid_width ||
      new_head.y >= (int16_t) snake->grid_height) {
    snake->alive = false;
    return;
  }
  if (snake_position_hits_body_without_tail(snake, new_head)) {
    snake->alive = false;
    return;
  }

  for (uint16_t i = snake->length - 1; i > 0; i--)
    snake->body[i] = snake->body[i - 1];

  snake->body[0] = new_head;
}

void snake_move_ignoring_self(snake_t *snake) {
  if (snake == NULL)
    return;

  if (!snake->alive)
    return;

  if (snake->length == 0)
    return;

  snake->current_direction = snake->next_direction;

  position_t new_head = snake_next_head_position(snake->body[0], snake->current_direction);

  if (new_head.x < 0 || new_head.y < 0 ||
      new_head.x >= (int16_t) snake->grid_width ||
      new_head.y >= (int16_t) snake->grid_height) {
    snake->alive = false;
    return;
  }

  for (uint16_t i = snake->length - 1; i > 0; i--)
    snake->body[i] = snake->body[i - 1];

  snake->body[0] = new_head;
}

bool snake_direction_would_be_fatal(const snake_t *snake, direction_t direction) {
  if (snake == NULL)
    return true;

  if (snake->length == 0)
    return true;

  if (!snake_direction_is_valid(direction))
    return true;

  position_t new_head = snake_next_head_position(snake->body[0], direction);

  if (new_head.x < 0 || new_head.y < 0 ||
      new_head.x >= (int16_t) snake->grid_width ||
      new_head.y >= (int16_t) snake->grid_height)
    return true;

  if (snake_position_hits_body_without_tail(snake, new_head))
    return true;

  return false;
}

void snake_grow(snake_t *snake) {
  if (snake == NULL)
    return;

  if (!snake->alive)
    return;

  if (snake->length == 0 || snake->length >= SNAKE_MAX_SEGMENTS)
    return;

  snake->body[snake->length] = snake->body[snake->length - 1];
  snake->length++;
}

uint16_t snake_get_length(const snake_t *snake) {
  if (snake == NULL)
    return 0;

  return snake->length;
}

position_t snake_get_segment(const snake_t *snake, uint16_t index) {
  position_t empty = {0, 0};

  if (snake == NULL)
    return empty;

  if (index >= snake->length)
    return empty;

  return snake->body[index];
}

position_t snake_get_head(const snake_t *snake) {
  return snake_get_segment(snake, 0);
}

direction_t snake_get_current_direction(const snake_t *snake) {
  if (snake == NULL)
    return DIRECTION_RIGHT;

  return snake->current_direction;
}

direction_t snake_get_next_direction(const snake_t *snake) {
  if (snake == NULL)
    return DIRECTION_RIGHT;

  return snake->next_direction;
}

bool snake_is_alive(const snake_t *snake) {
  if (snake == NULL)
    return false;

  return snake->alive;
}
