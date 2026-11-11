#include <lcom/lcf.h>
#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "i8042.h"
#include "kbc.h"
#include "timer.h"

static int print_scancode_if_ready(bool *esc_pressed) {
  if (esc_pressed == NULL)
    return 1;

  if (!kbc_scancode_ready())
    return 0;

  uint8_t bytes[2];
  uint8_t size = kbc_get_scancode_size();
  bool make = kbc_scancode_is_make();

  kbc_get_scancode_bytes(bytes);

  if (kbd_print_scancode(make, size, bytes) != 0)
    return 1;

  *esc_pressed = (size == 1 && bytes[0] == ESC_BREAK);

  kbc_clear_scancode();

  return 0;
}

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)(void) {
  uint8_t bit_no;
  uint32_t irq_set;
  int ipc_status;
  message msg;
  bool esc_pressed = false;

  if (kbd_subscribe_int(&bit_no) != 0)
    return 1;

  irq_set = BIT(bit_no);

  while (!esc_pressed) {
    int r = driver_receive(ANY, &msg, &ipc_status);

    if (r != OK) {
      printf("driver_receive failed with: %d\n", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();

            if (kbc_get_error())
              continue;

            if (print_scancode_if_ready(&esc_pressed) != 0) {
              kbd_unsubscribe_int();
              return 1;
            }
          }
          break;

        default:
          break;
      }
    }
  }

  if (kbd_unsubscribe_int() != 0)
    return 1;

  return 0;
}

int(kbd_test_poll)(void) {
  uint8_t data;
  bool esc_pressed = false;
  int ret = 0;

#ifdef LAB3
  kbc_reset_sys_inb_count();
#endif

  while (!esc_pressed) {
    if (kbc_read_outbuf(&data) == 0) {
      kbc_process_scancode_byte(data);

      if (print_scancode_if_ready(&esc_pressed) != 0) {
        ret = 1;
        break;
      }
    }

    tickdelay(micros_to_ticks(DELAY_US));
  }

  if (kbc_set_keyboard_interrupts(true) != 0)
    ret = 1;

#ifdef LAB3
  if (ret == 0)
    kbd_print_no_sysinb(kbc_get_sys_inb_count());
#endif

  return ret;
}

int(kbd_test_timed_scan)(uint8_t n) {
  uint8_t kbd_bit_no;
  uint8_t timer_bit_no;
  uint32_t irq_set_kbd;
  uint32_t irq_set_timer;
  uint32_t idle_limit_ticks = (uint32_t) n * sys_hz();

  int ipc_status;
  message msg;

  bool esc_pressed = false;
  bool kbd_subscribed = false;
  bool timer_subscribed = false;
  int ret = 0;

  if (kbd_subscribe_int(&kbd_bit_no) != 0)
    return 1;

  kbd_subscribed = true;
  irq_set_kbd = BIT(kbd_bit_no);

  if (timer_subscribe_int(&timer_bit_no) != 0) {
    ret = 1;
    goto cleanup;
  }

  timer_subscribed = true;
  irq_set_timer = BIT(timer_bit_no);

  timer_reset_counter();

  while (!esc_pressed && timer_get_counter() < idle_limit_ticks) {
    int r = driver_receive(ANY, &msg, &ipc_status);

    if (r != OK) {
      printf("driver_receive failed with: %d\n", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set_kbd) {
            kbc_ih();

            if (!kbc_get_error()) {
              timer_reset_counter();

              if (print_scancode_if_ready(&esc_pressed) != 0) {
                ret = 1;
                goto cleanup;
              }
            }
          }

          if (msg.m_notify.interrupts & irq_set_timer) {
            timer_int_handler();
          }
          break;

        default:
          break;
      }
    }
  }

cleanup:
  if (timer_subscribed) {
    if (timer_unsubscribe_int() != 0)
      ret = 1;
  }

  if (kbd_subscribed) {
    if (kbd_unsubscribe_int() != 0)
      ret = 1;
  }

  return ret;
}
