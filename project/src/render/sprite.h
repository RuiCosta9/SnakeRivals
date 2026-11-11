#ifndef SPRITE_H
#define SPRITE_H

#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @file sprite.h
 * @brief Sprite abstraction over XPM pixmaps.
 *
 * Sprites own decoded pixel buffers and optional transparency metadata used by
 * the renderer and asset manager.
 */

/**
 * @brief Opaque decoded sprite.
 */
typedef struct sprite sprite_t;

/**
 * @brief Create a sprite from an XPM map.
 *
 * @param xpm XPM source map.
 * @param type XPM image type used by LCF.
 * @param transparent_color Color key used for transparency.
 * @param use_transparency Whether transparent pixels should be skipped.
 * @return New sprite, or NULL on failure.
 */
sprite_t *sprite_create(xpm_map_t xpm, enum xpm_image_type type, uint32_t transparent_color, bool use_transparency);

/**
 * @brief Create a recolored sprite from an XPM map.
 *
 * @param xpm XPM source map.
 * @param type XPM image type used by LCF.
 * @param transparent_color Color key used for transparency.
 * @param use_transparency Whether transparent pixels should be skipped.
 * @param color_map Palette remapping table.
 * @return New sprite, or NULL on failure.
 */
sprite_t *sprite_create_recolored(xpm_map_t xpm, enum xpm_image_type type, uint32_t transparent_color, bool use_transparency, const uint8_t color_map[256]);

/**
 * @brief Destroy a sprite and release its pixel buffer.
 *
 * @param sprite Sprite to destroy.
 */
void sprite_destroy(sprite_t *sprite);

/**
 * @brief Draw a sprite immediately.
 *
 * @param sprite Sprite to draw.
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @return 0 on success, non-zero on failure.
 */
int sprite_draw(const sprite_t *sprite, uint16_t x, uint16_t y);

/** @brief Get the raw pixel buffer. @param sprite Sprite to inspect. @return Pixel buffer or NULL. */
const uint8_t *sprite_get_pixels(const sprite_t *sprite);

/** @brief Get sprite width. @param sprite Sprite to inspect. @return Width in pixels. */
uint16_t sprite_get_width(const sprite_t *sprite);

/** @brief Get sprite height. @param sprite Sprite to inspect. @return Height in pixels. */
uint16_t sprite_get_height(const sprite_t *sprite);

/** @brief Get pixel buffer size. @param sprite Sprite to inspect. @return Size in bytes. */
size_t sprite_get_size(const sprite_t *sprite);

/** @brief Get transparent color key. @param sprite Sprite to inspect. @return Color key. */
uint32_t sprite_get_transparent_color(const sprite_t *sprite);

/** @brief Check whether transparency is enabled. @param sprite Sprite to inspect. @return true if enabled. */
bool sprite_uses_transparency(const sprite_t *sprite);

#endif
