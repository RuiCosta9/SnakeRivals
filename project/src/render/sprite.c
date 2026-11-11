/**
 * @file sprite.c
 * @brief Decoded sprite allocation, drawing and metadata access.
 */

#include "sprite.h"

#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include "video.h"

struct sprite {
  uint8_t *pixels;
  uint16_t width;
  uint16_t height;
  size_t size;
  uint32_t transparent_color;
  bool use_transparency;
};

sprite_t *sprite_create(xpm_map_t xpm, enum xpm_image_type type, uint32_t transparent_color, bool use_transparency) {
  if (xpm == NULL)
    return NULL;

  sprite_t *sprite = (sprite_t *) malloc(sizeof(sprite_t));

  if (sprite == NULL)
    return NULL;

  xpm_image_t image;

  sprite->pixels = xpm_load(xpm, type, &image);

  if (sprite->pixels == NULL) {
    free(sprite);
    return NULL;
  }

  sprite->width = image.width;
  sprite->height = image.height;
  sprite->size = image.size;
  sprite->transparent_color = transparent_color;
  sprite->use_transparency = use_transparency;

  return sprite;
}

sprite_t *sprite_create_recolored(xpm_map_t xpm, enum xpm_image_type type, uint32_t transparent_color, bool use_transparency, const uint8_t color_map[256]) {
  sprite_t *sprite = sprite_create(xpm, type, transparent_color, use_transparency);

  if (sprite == NULL)
    return NULL;

  if (color_map != NULL) {
    for (size_t i = 0; i < sprite->size; i++)
      sprite->pixels[i] = color_map[sprite->pixels[i]];
  }

  return sprite;
}

void sprite_destroy(sprite_t *sprite) {
  if (sprite == NULL)
    return;

  if (sprite->pixels != NULL)
    free(sprite->pixels);

  free(sprite);
}

int sprite_draw(const sprite_t *sprite, uint16_t x, uint16_t y) {
  if (sprite == NULL)
    return 1;

  return video_draw_pixmap(sprite->pixels, sprite->width, sprite->height, x, y, sprite->transparent_color, sprite->use_transparency);
}

const uint8_t *sprite_get_pixels(const sprite_t *sprite) {
  if (sprite == NULL)
    return NULL;

  return sprite->pixels;
}

uint16_t sprite_get_width(const sprite_t *sprite) {
  if (sprite == NULL)
    return 0;

  return sprite->width;
}

uint16_t sprite_get_height(const sprite_t *sprite) {
  if (sprite == NULL)
    return 0;

  return sprite->height;
}

size_t sprite_get_size(const sprite_t *sprite) {
  if (sprite == NULL)
    return 0;

  return sprite->size;
}

uint32_t sprite_get_transparent_color(const sprite_t *sprite) {
  if (sprite == NULL)
    return 0;

  return sprite->transparent_color;
}

bool sprite_uses_transparency(const sprite_t *sprite) {
  if (sprite == NULL)
    return false;

  return sprite->use_transparency;
}
