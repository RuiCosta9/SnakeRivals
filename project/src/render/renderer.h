#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

#include "sprite.h"
#include "state.h"

/**
 * @file renderer.h
 * @brief Public renderer API and render-layer definitions.
 *
 * The renderer collects drawing commands, sorts them by layer/order and presents
 * a complete frame using the video subsystem.
 */

/**
 * @brief Logical z-layers used by renderer command submission.
 */
typedef enum {
  RENDER_LAYER_BACKGROUND = 0, /**< Background fills and images. */
  RENDER_LAYER_WORLD = 100,    /**< Arena/world decorations. */
  RENDER_LAYER_FOOD = 200,     /**< Food items. */
  RENDER_LAYER_POWER_UP = 300, /**< Powerup items and icons. */
  RENDER_LAYER_SNAKE_BODY = 400, /**< Snake body cells. */
  RENDER_LAYER_SNAKE_HEAD = 500, /**< Snake heads. */
  RENDER_LAYER_UI = 600,       /**< Menus, HUD and overlays. */
  RENDER_LAYER_CURSOR = 700    /**< Mouse cursor, always on top. */
} render_layer_t;

/**
 * @brief Initialize renderer state and triple buffering.
 *
 * @return 0 on success, non-zero on failure.
 */
int renderer_init(void);

/**
 * @brief Shutdown renderer state and disable triple buffering.
 *
 * @return 0 on success, non-zero on failure.
 */
int renderer_shutdown(void);

/**
 * @brief Start a new frame and clear the draw buffer.
 *
 * @param clear_color Background color.
 * @return 0 on success, non-zero on failure.
 */
int renderer_begin_frame(uint32_t clear_color);

/**
 * @brief Draw all queued commands and present the frame.
 *
 * @return 0 on success, non-zero on failure.
 */
int renderer_flush(void);

/**
 * @brief Clear the active draw buffer.
 *
 * @param color Fill color.
 * @return 0 on success, non-zero on failure.
 */
int renderer_clear(uint32_t color);

/**
 * @brief Present the active draw buffer.
 *
 * @return 0 on success, non-zero on failure.
 */
int renderer_present(void);

/**
 * @brief Draw a rectangle immediately.
 *
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @param width Rectangle width.
 * @param height Rectangle height.
 * @param color Fill color.
 * @return 0 on success, non-zero on failure.
 */
int renderer_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

/**
 * @brief Draw a sprite immediately.
 *
 * @param sprite Sprite to draw.
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @return 0 on success, non-zero on failure.
 */
int renderer_draw_sprite(const sprite_t *sprite, uint16_t x, uint16_t y);

/**
 * @brief Submit a rectangle command for the current frame.
 *
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @param width Rectangle width.
 * @param height Rectangle height.
 * @param color Fill color.
 * @param z Render layer/order key.
 * @return 0 on success, non-zero if the command queue is full.
 */
int renderer_submit_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, int16_t z);

/**
 * @brief Submit a sprite command for the current frame.
 *
 * @param sprite Sprite to draw.
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @param z Render layer/order key.
 * @return 0 on success, non-zero if the command queue is full.
 */
int renderer_submit_sprite(const sprite_t *sprite, uint16_t x, uint16_t y, int16_t z);

/**
 * @brief Submit a text command for the current frame.
 *
 * @param text Text to draw.
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @param scale Pixel font scale.
 * @param color Text color.
 * @param z Render layer/order key.
 * @return 0 on success, non-zero if the command queue is full.
 */
int renderer_submit_text(const char *text, uint16_t x, uint16_t y, uint8_t scale, uint32_t color, int16_t z);

/**
 * @brief Render a specific application state.
 *
 * @param state State to draw.
 * @return 0 on success, non-zero on failure.
 */
int renderer_render_state(app_state_t state);

/**
 * @brief Render the currently active application state.
 *
 * @return 0 on success, non-zero on failure.
 */
int renderer_render_current_state(void);

#endif
