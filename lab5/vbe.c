#include <lcom/lcf.h>
#include <lcom/vbe.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "vbe.h"
#include "vbe_defs.h"

static vbe_mode_info_t mode_info;

static uint8_t *frame_buffer = NULL;
static uint8_t *buffers[VBE_TRIPLE_BUFFER_COUNT] = {NULL, NULL, NULL};

static unsigned int frame_buffer_size = 0;
static unsigned int mapped_buffer_size = 0;

static uint8_t bytes_per_pixel = 0;
static uint16_t bytes_per_scanline = 0;

static uint8_t front_buffer_index = 0;
static uint8_t draw_buffer_index = 0;
static uint8_t spare_buffer_index = 1;

static bool triple_buffering_enabled = false;

static void vbe_reset_buffer_state(void) {
  front_buffer_index = 0;
  draw_buffer_index = 0;
  spare_buffer_index = 1;
  triple_buffering_enabled = false;
}

static void vbe_setup_buffer_pointers(void) {
  for (uint8_t i = 0; i < VBE_TRIPLE_BUFFER_COUNT; i++)
    buffers[i] = frame_buffer + i * frame_buffer_size;

  vbe_reset_buffer_state();
}

int vbe_map(uint16_t mode) {
  if (vbe_get_mode_info(mode, &mode_info) != OK) {
    printf("%s: vbe_get_mode_info() failed\n", __func__);
    return 1;
  }

  bytes_per_pixel = (mode_info.BitsPerPixel + 7) / 8;

  bytes_per_scanline = mode_info.LinBytesPerScanLine;

  if (bytes_per_scanline == 0)
    bytes_per_scanline = mode_info.BytesPerScanLine;

  if (bytes_per_pixel == 0 || bytes_per_scanline == 0)
    return 1;

  frame_buffer_size = mode_info.YResolution * bytes_per_scanline;
  mapped_buffer_size = frame_buffer_size * VBE_TRIPLE_BUFFER_COUNT;

  struct minix_mem_range mr;
  mr.mr_base = (phys_bytes) mode_info.PhysBasePtr;
  mr.mr_limit = mr.mr_base + mapped_buffer_size;

  int r;

  if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr))) {
    printf("%s: sys_privctl(SYS_PRIV_ADD_MEM) failed: %d\n", __func__, r);
    return 1;
  }

  frame_buffer = vm_map_phys(SELF, (void *) mr.mr_base, mapped_buffer_size);

  if (frame_buffer == MAP_FAILED) {
    printf("%s: vm_map_phys() failed\n", __func__);
    frame_buffer = NULL;
    return 1;
  }

  vbe_setup_buffer_pointers();

  return 0;
}

int vbe_set_mode(uint16_t mode) {
  reg86_t r;

  memset(&r, 0, sizeof(r));

  r.ax = VBE_FUNC_SET_MODE;
  r.bx = VBE_LINEAR_FRAMEBUFFER | mode;
  r.intno = VBE_INTNO;

  if (sys_int86(&r) != OK)
    return 1;

  if (r.al != VBE_SUCCESS_AL || r.ah != VBE_SUCCESS_AH)
    return 1;

  return 0;
}

int vbe_set_display_start(uint16_t x, uint16_t y, bool wait_for_vertical_retrace) {
  reg86_t r;

  memset(&r, 0, sizeof(r));

  r.ax = VBE_FUNC_SET_DISPLAY_START;
  r.bx = wait_for_vertical_retrace ? VBE_SET_DISPLAY_START_VERTICAL_RETRACE : VBE_SET_DISPLAY_START;
  r.cx = x;
  r.dx = y;
  r.intno = VBE_INTNO;

  if (sys_int86(&r) != OK)
    return 1;

  if (r.al != VBE_SUCCESS_AL || r.ah != VBE_SUCCESS_AH)
    return 1;

  return 0;
}

int vbe_init(uint16_t mode) {
  if (vbe_map(mode) != 0)
    return 1;

  if (vbe_set_mode(mode) != 0)
    return 1;

  memset(frame_buffer, 0, mapped_buffer_size);

  if (vbe_set_display_start(0, 0, false) != 0)
    return 1;

  return 0;
}

int vbe_enable_triple_buffering(void) {
  if (frame_buffer == NULL)
    return 1;

  if (frame_buffer_size == 0)
    return 1;

  front_buffer_index = 0;
  draw_buffer_index = 1;
  spare_buffer_index = 2;
  triple_buffering_enabled = true;

  return 0;
}

int vbe_disable_triple_buffering(void) {
  if (frame_buffer != NULL) {
    if (vbe_set_display_start(0, 0, false) != 0)
      return 1;
  }

  vbe_reset_buffer_state();

  return 0;
}

bool vbe_is_triple_buffering_enabled(void) {
  return triple_buffering_enabled;
}

int vbe_present_draw_buffer(void) {
  if (!triple_buffering_enabled)
    return 0;

  uint8_t old_front_buffer_index = front_buffer_index;
  uint8_t new_front_buffer_index = draw_buffer_index;

  uint16_t display_start_y = new_front_buffer_index * mode_info.YResolution;
  
  if (vbe_set_display_start(0, display_start_y, false) != 0)
    return 1;

  front_buffer_index = new_front_buffer_index;
  draw_buffer_index = spare_buffer_index;
  spare_buffer_index = old_front_buffer_index;

  return 0;
}

int vbe_exit(void) {
  if (frame_buffer != NULL)
    vbe_set_display_start(0, 0, false);

  vbe_reset_buffer_state();

  frame_buffer = NULL;

  for (uint8_t i = 0; i < VBE_TRIPLE_BUFFER_COUNT; i++)
    buffers[i] = NULL;

  frame_buffer_size = 0;
  mapped_buffer_size = 0;
  bytes_per_pixel = 0;
  bytes_per_scanline = 0;

  return vg_exit();
}

void *vbe_get_frame_buffer(void) {
  return frame_buffer;
}

void *vbe_get_visible_buffer(void) {
  if (frame_buffer == NULL)
    return NULL;

  return buffers[front_buffer_index];
}

void *vbe_get_draw_buffer(void) {
  if (frame_buffer == NULL)
    return NULL;

  return buffers[draw_buffer_index];
}

uint16_t vbe_get_hres(void) {
  return mode_info.XResolution;
}

uint16_t vbe_get_vres(void) {
  return mode_info.YResolution;
}

uint8_t vbe_get_bytes_per_pixel(void) {
  return bytes_per_pixel;
}

uint16_t vbe_get_bytes_per_scanline(void) {
  return bytes_per_scanline;
}

unsigned int vbe_get_frame_buffer_size(void) {
  return frame_buffer_size;
}

uint8_t vbe_get_front_buffer_index(void) {
  return front_buffer_index;
}

uint8_t vbe_get_draw_buffer_index(void) {
  return draw_buffer_index;
}
