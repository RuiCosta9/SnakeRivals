// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "video.h"
#include "vbe.h"
#include "kbc.h"
#include "i8042.h"

static int wait_for_esc(void);

static uint32_t kbd_irq_set;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

static int wait_for_esc(void) {
  int ipc_status;
  message msg;
  bool done = false;

  kbc_reset_scancode();
  kbc_clear_scancode();

  while (!done) {
    int r = driver_receive(ANY, &msg, &ipc_status);

    if (r != OK)
      continue;

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & kbd_irq_set) {
            kbc_ih();

            if (!kbc_get_error() && kbc_get_scancode() == ESC_BREAK)
              done = true;
          }
          break;

        default:
          break;
      }
    }
  }

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
  if (vbe_set_mode(mode) != 0)
    return 1;

  sleep(delay);

  if (vbe_exit() != 0)
    return 1;

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {
  uint8_t bit_no;
  int ret = 0;

  if (kbd_subscribe_int(&bit_no) != 0)
    return 1;

  kbd_irq_set = BIT(bit_no);

  if (vbe_init(mode) != 0) {
    kbd_unsubscribe_int();
    return 1;
  }

  if (video_draw_rectangle(x, y, width, height, color) != 0)
    ret = 1;

  if (ret == 0 && wait_for_esc() != 0)
    ret = 1;

  if (vbe_exit() != 0)
    ret = 1;

  if (kbd_unsubscribe_int() != 0)
    ret = 1;

  return ret;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  uint8_t bit_no;
  int ret = 0;

  if (kbd_subscribe_int(&bit_no) != 0)
    return 1;

  kbd_irq_set = BIT(bit_no);

  if (vbe_init(0x105) != 0) {
    kbd_unsubscribe_int();
    return 1;
  }

  if (video_draw_xpm(xpm, x, y) != 0)
    ret = 1;

  if (ret == 0 && wait_for_esc() != 0)
    ret = 1;

  if (vbe_exit() != 0)
    ret = 1;

  if (kbd_unsubscribe_int() != 0)
    ret = 1;

  return ret;
}
