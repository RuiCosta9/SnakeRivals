#ifndef VIDEO_H
#define VIDEO_H

#include <lcom/lcf.h>
#include <lcom/vbe.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * @file video.h
 * @brief Low-level pixel and pixmap drawing helpers reused by the renderer.
 */

/** @brief Draw one pixel. @param x X coordinate. @param y Y coordinate. @param color Pixel color. @return 0 on success. */
int video_draw_pixel(uint16_t x, uint16_t y, uint32_t color);

/** @brief Draw a horizontal line. @param x X coordinate. @param y Y coordinate. @param len Line length. @param color Pixel color. @return 0 on success. */
int video_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

/** @brief Draw a filled rectangle. @param x X coordinate. @param y Y coordinate. @param width Width. @param height Height. @param color Fill color. @return 0 on success. */
int video_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

/**
 * @brief Draw a decoded pixmap.
 *
 * @param pixmap Pixel buffer.
 * @param width Width in pixels.
 * @param height Height in pixels.
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @param transparent_color Color key.
 * @param use_transparency Whether transparent pixels are skipped.
 * @return 0 on success.
 */
int video_draw_pixmap(const uint8_t *pixmap, uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint32_t transparent_color, bool use_transparency);

/** @brief Draw an XPM directly. @param xpm XPM map. @param x Left coordinate. @param y Top coordinate. @return 0 on success. */
int video_draw_xpm(xpm_map_t xpm, uint16_t x, uint16_t y);

/** @brief Clear the active draw buffer. @param color Fill color. @return 0 on success. */
int video_clear_screen(uint32_t color);

#endif
