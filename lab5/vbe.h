#ifndef CUSTOM_VBE_H
#define CUSTOM_VBE_H

#include <lcom/lcf.h>
#include <lcom/vbe.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * @file vbe.h
 * @brief VBE graphics mode and buffering helpers reused by the project.
 */

/** @brief Initialize VBE graphics mode. @param mode VBE mode. @return 0 on success. */
int vbe_init(uint16_t mode);

/** @brief Map VBE frame buffer memory. @param mode VBE mode. @return 0 on success. */
int vbe_map(uint16_t mode);

/** @brief Set VBE graphics mode. @param mode VBE mode. @return 0 on success. */
int vbe_set_mode(uint16_t mode);

/** @brief Leave graphics mode. @return 0 on success. */
int vbe_exit(void);

/** @brief Enable triple buffering. @return 0 on success. */
int vbe_enable_triple_buffering(void);

/** @brief Disable triple buffering. @return 0 on success. */
int vbe_disable_triple_buffering(void);

/** @brief Check whether triple buffering is active. @return true if enabled. */
bool vbe_is_triple_buffering_enabled(void);

/** @brief Set visible display start. @param x X offset. @param y Y offset. @param wait_for_vertical_retrace Whether to wait for retrace. @return 0 on success. */
int vbe_set_display_start(uint16_t x, uint16_t y, bool wait_for_vertical_retrace);

/** @brief Present the current draw buffer. @return 0 on success. */
int vbe_present_draw_buffer(void);

/** @brief Get mapped frame-buffer base. @return Frame-buffer pointer. */
void *vbe_get_frame_buffer(void);

/** @brief Get visible buffer pointer. @return Visible buffer pointer. */
void *vbe_get_visible_buffer(void);

/** @brief Get draw buffer pointer. @return Draw buffer pointer. */
void *vbe_get_draw_buffer(void);

/** @brief Get horizontal resolution. @return Width in pixels. */
uint16_t vbe_get_hres(void);

/** @brief Get vertical resolution. @return Height in pixels. */
uint16_t vbe_get_vres(void);

/** @brief Get bytes per pixel. @return Bytes per pixel. */
uint8_t vbe_get_bytes_per_pixel(void);

/** @brief Get bytes per scanline. @return Bytes per scanline. */
uint16_t vbe_get_bytes_per_scanline(void);

/** @brief Get mapped frame-buffer size. @return Size in bytes. */
unsigned int vbe_get_frame_buffer_size(void);

/** @brief Get visible/front buffer index. @return Buffer index. */
uint8_t vbe_get_front_buffer_index(void);

/** @brief Get current draw buffer index. @return Buffer index. */
uint8_t vbe_get_draw_buffer_index(void);

#endif
