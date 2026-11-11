#include <lcom/lcf.h>
#include <lcom/vbe.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "video.h"
#include "vbe.h"

static uint8_t *video_get_draw_ptr(uint16_t x, uint16_t y) {
  uint8_t *target_buffer = vbe_get_draw_buffer();

  if (target_buffer == NULL)
    return NULL;

  if (x >= vbe_get_hres() || y >= vbe_get_vres())
    return NULL;

  return target_buffer +
         (uint32_t) y * vbe_get_bytes_per_scanline() +
         (uint32_t) x * vbe_get_bytes_per_pixel();
}

int video_draw_pixel(uint16_t x, uint16_t y, uint32_t color) {
  uint8_t *ptr = video_get_draw_ptr(x, y);

  if (ptr == NULL)
    return 1;

  memcpy(ptr, &color, vbe_get_bytes_per_pixel());

  return 0;
}

int video_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  if (len == 0)
    return 0;

  if (y >= vbe_get_vres() || x >= vbe_get_hres())
    return 1;

  if ((uint32_t) x + len > vbe_get_hres())
    return 1;

  uint8_t *ptr = video_get_draw_ptr(x, y);

  if (ptr == NULL)
    return 1;

  uint8_t bytes_per_pixel = vbe_get_bytes_per_pixel();

  if (bytes_per_pixel == 1) {
    memset(ptr, (uint8_t) color, len);
    return 0;
  }

  for (uint16_t i = 0; i < len; i++)
    memcpy(ptr + (uint32_t) i * bytes_per_pixel, &color, bytes_per_pixel);

  return 0;
}

int video_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  if (width == 0 || height == 0)
    return 0;

  if (x >= vbe_get_hres() || y >= vbe_get_vres())
    return 1;

  if ((uint32_t) x + width > vbe_get_hres())
    return 1;

  if ((uint32_t) y + height > vbe_get_vres())
    return 1;

  uint8_t *target_buffer = vbe_get_draw_buffer();

  if (target_buffer == NULL)
    return 1;

  uint8_t bytes_per_pixel = vbe_get_bytes_per_pixel();
  uint16_t bytes_per_scanline = vbe_get_bytes_per_scanline();
  uint32_t row_size = (uint32_t) width * bytes_per_pixel;
  uint8_t *first_row = target_buffer +
                       (uint32_t) y * bytes_per_scanline +
                       (uint32_t) x * bytes_per_pixel;

  if (bytes_per_pixel == 1) {
    for (uint16_t row = 0; row < height; row++) {
      uint8_t *row_ptr = first_row + (uint32_t) row * bytes_per_scanline;
      memset(row_ptr, (uint8_t) color, width);
    }

    return 0;
  }

  for (uint16_t col = 0; col < width; col++)
    memcpy(first_row + (uint32_t) col * bytes_per_pixel, &color, bytes_per_pixel);

  for (uint16_t row = 1; row < height; row++) {
    uint8_t *row_ptr = first_row + (uint32_t) row * bytes_per_scanline;
    memcpy(row_ptr, first_row, row_size);
  }

  return 0;
}

static void video_draw_opaque_pixmap_rows(const uint8_t *pixmap, uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t bytes_per_pixel, uint16_t bytes_per_scanline, uint8_t *target_buffer) {
  uint32_t row_size = (uint32_t) width * bytes_per_pixel;

  for (uint16_t row = 0; row < height; row++) {
    uint8_t *dst = target_buffer +
                   (uint32_t) (y + row) * bytes_per_scanline +
                   (uint32_t) x * bytes_per_pixel;
    const uint8_t *src = pixmap + (uint32_t) row * row_size;

    memcpy(dst, src, row_size);
  }
}

static void video_draw_transparent_pixmap_8bpp(const uint8_t *pixmap, uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t transparent_color, uint16_t bytes_per_scanline, uint8_t *target_buffer) {
  for (uint16_t row = 0; row < height; row++) {
    uint8_t *dst = target_buffer + (uint32_t) (y + row) * bytes_per_scanline + x;
    const uint8_t *src = pixmap + (uint32_t) row * width;
    uint16_t col = 0;

    while (col < width) {
      while (col < width && src[col] == transparent_color)
        col++;

      uint16_t start = col;

      while (col < width && src[col] != transparent_color)
        col++;

      if (col > start)
        memcpy(dst + start, src + start, col - start);
    }
  }
}

static void video_draw_transparent_pixmap_generic(const uint8_t *pixmap, uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint32_t transparent_color, uint8_t bytes_per_pixel, uint16_t bytes_per_scanline, uint8_t *target_buffer) {
  uint8_t transparent_bytes[4] = {0, 0, 0, 0};

  memcpy(transparent_bytes, &transparent_color, bytes_per_pixel);

  for (uint16_t row = 0; row < height; row++) {
    uint8_t *dst = target_buffer +
                   (uint32_t) (y + row) * bytes_per_scanline +
                   (uint32_t) x * bytes_per_pixel;
    const uint8_t *src = pixmap +
                         (uint32_t) row * width * bytes_per_pixel;

    for (uint16_t col = 0; col < width; col++) {
      const uint8_t *src_pixel = src + (uint32_t) col * bytes_per_pixel;

      if (memcmp(src_pixel, transparent_bytes, bytes_per_pixel) == 0)
        continue;

      memcpy(dst + (uint32_t) col * bytes_per_pixel, src_pixel, bytes_per_pixel);
    }
  }
}

int video_draw_pixmap(const uint8_t *pixmap, uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint32_t transparent_color, bool use_transparency) {
  if (pixmap == NULL)
    return 1;

  if (width == 0 || height == 0)
    return 0;

  if (x >= vbe_get_hres() || y >= vbe_get_vres())
    return 1;

  if ((uint32_t) x + width > vbe_get_hres())
    return 1;

  if ((uint32_t) y + height > vbe_get_vres())
    return 1;

  uint8_t *target_buffer = vbe_get_draw_buffer();

  if (target_buffer == NULL)
    return 1;

  uint8_t bytes_per_pixel = vbe_get_bytes_per_pixel();
  uint16_t bytes_per_scanline = vbe_get_bytes_per_scanline();

  if (!use_transparency) {
    video_draw_opaque_pixmap_rows(pixmap, width, height, x, y, bytes_per_pixel, bytes_per_scanline, target_buffer);
    return 0;
  }

  if (bytes_per_pixel == 1) {
    video_draw_transparent_pixmap_8bpp(pixmap, width, height, x, y, (uint8_t) transparent_color, bytes_per_scanline, target_buffer);
    return 0;
  }

  video_draw_transparent_pixmap_generic(pixmap, width, height, x, y, transparent_color, bytes_per_pixel, bytes_per_scanline, target_buffer);

  return 0;
}

int video_draw_xpm(xpm_map_t xpm, uint16_t x, uint16_t y) {
  xpm_image_t img;
  uint8_t *pixmap = xpm_load(xpm, XPM_INDEXED, &img);

  if (pixmap == NULL)
    return 1;

  int ret = video_draw_pixmap(pixmap, img.width, img.height, x, y, 0, false);

  free(pixmap);

  return ret;
}

int video_clear_screen(uint32_t color) {
  uint8_t *target_buffer = vbe_get_draw_buffer();

  if (target_buffer == NULL)
    return 1;

  if (color == 0) {
    memset(target_buffer, 0, vbe_get_frame_buffer_size());
    return 0;
  }

  if (vbe_get_bytes_per_pixel() == 1) {
    memset(target_buffer, (uint8_t) color, vbe_get_frame_buffer_size());
    return 0;
  }

  return video_draw_rectangle(0, 0, vbe_get_hres(), vbe_get_vres(), color);
}
