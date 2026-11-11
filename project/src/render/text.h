#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>

/**
 * @file text.h
 * @brief Small bitmap-font text drawing helpers.
 */

#define TEXT_FONT_WIDTH 3
#define TEXT_FONT_HEIGHT 5
#define TEXT_FONT_SPACING 1

/**
 * @brief Compute rendered text width.
 *
 * @param text Null-terminated text.
 * @param scale Pixel scale.
 * @return Width in pixels.
 */
uint16_t text_width(const char *text, uint8_t scale);

/**
 * @brief Draw text immediately using the built-in bitmap font.
 *
 * @param text Null-terminated text.
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @param scale Pixel scale.
 * @param color Text color.
 * @return 0 on success, non-zero on failure.
 */
int text_draw(const char *text, uint16_t x, uint16_t y, uint8_t scale, uint32_t color);

#endif
