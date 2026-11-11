/**
 * @file arena_walls.c
 * @brief Static Arena Battle wall layout implementation.
 */

#include "arena_walls.h"

#include <stddef.h>

typedef struct {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
} arena_wall_block_t;


static const arena_wall_block_t arena_wall_blocks[] = {
  {18, 20, 12, 3},
  {40, 14, 3, 11},
  {60, 22, 11, 3},
  {74, 30, 3, 12},
  {30, 33, 11, 3},
  {50, 40, 3, 11},
  {20, 45, 10, 3},
  {64, 50, 12, 3},
  {44, 52, 3, 10},
  {12, 32, 3, 10},
};

#define ARENA_WALL_BLOCK_COUNT (sizeof(arena_wall_blocks) / sizeof(arena_wall_blocks[0]))

uint8_t arena_walls_get_count(void) {
  return (uint8_t) ARENA_WALL_BLOCK_COUNT;
}

void arena_walls_get_block(uint8_t index, int16_t *x, int16_t *y, int16_t *w, int16_t *h) {
  if (index >= ARENA_WALL_BLOCK_COUNT)
    return;

  if (x != NULL) *x = arena_wall_blocks[index].x;
  if (y != NULL) *y = arena_wall_blocks[index].y;
  if (w != NULL) *w = arena_wall_blocks[index].w;
  if (h != NULL) *h = arena_wall_blocks[index].h;
}

bool arena_walls_cell_is_wall(int16_t cell_x, int16_t cell_y) {
  for (uint8_t i = 0; i < ARENA_WALL_BLOCK_COUNT; i++) {
    const arena_wall_block_t *block = &arena_wall_blocks[i];

    if (cell_x >= block->x && cell_x < block->x + block->w &&
        cell_y >= block->y && cell_y < block->y + block->h)
      return true;
  }

  return false;
}
