/**
 * @file app.c
 * @brief Application bootstrap, interrupt loop and resource cleanup.
 */

#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app.h"

#include "assets.h"
#include "kbc.h"
#include "mouse.h"
#include "mouse_cursor.h"
#include "renderer.h"
#include "serial.h"
#include "state.h"
#include "timer.h"
#include "vbe.h"

#define APP_VIDEO_MODE 0x105

#define APP_TIMER_HZ 60
#define APP_TARGET_FPS 60
#define APP_TICKS_PER_FRAME (APP_TIMER_HZ / APP_TARGET_FPS)

static uint32_t timer_irq_set = 0;
static uint32_t kbd_irq_set = 0;
static uint32_t mouse_irq_set = 0;
static uint32_t serial_irq_set = 0;

static bool timer_subscribed = false;
static bool kbd_subscribed = false;
static bool mouse_subscribed = false;
static bool serial_initialized = false;
static bool serial_subscribed = false;
static bool serial_interrupts_enabled = false;
static bool mouse_data_reporting_enabled = false;
static bool graphics_initialized = false;
static bool renderer_initialized = false;
static bool assets_loaded = false;

static unsigned int keyboard_events = 0;
static unsigned int mouse_packets = 0;
static unsigned int serial_interrupts = 0;
static unsigned int rendered_frames = 0;
static unsigned int frame_tick_counter = 0;

static bool app_handle_timer_interrupt(void) {
  timer_int_handler();
  state_handle_timer_event();

  frame_tick_counter++;

  if (frame_tick_counter < APP_TICKS_PER_FRAME)
    return false;

  frame_tick_counter = 0;
  return true;
}

static void app_render_frame(void) {
  if (renderer_render_current_state() == 0)
    rendered_frames++;
}

static void app_handle_keyboard_interrupt(void) {
  kbc_ih();

  if (kbc_get_error())
    return;

  if (!kbc_scancode_ready())
    return;

  keyboard_events++;
  state_handle_keyboard_event(kbc_get_scancode());
}

static void app_handle_serial_interrupt(void) {
  serial_ih();
  serial_interrupts++;
  state_handle_serial_event();
}

static void app_handle_mouse_interrupt(void) {
  mouse_ih();

  if (mouse_get_error()) {
    mouse_reset_packet_state();
    return;
  }

  mouse_sync_packet(mouse_get_byte());

  if (mouse_packet_ready()) {
    struct packet packet;

    if (mouse_get_packet(&packet) == 0) {
      mouse_packets++;
      mouse_cursor_update(&packet);
      state_handle_mouse_event(&packet);
    }
  }
}

int app_init(void) {
  if (timer_subscribed || kbd_subscribed || mouse_subscribed || serial_initialized || graphics_initialized)
    return 1;

  state_init();

  keyboard_events = 0;
  mouse_packets = 0;
  serial_interrupts = 0;
  rendered_frames = 0;
  frame_tick_counter = 0;

  timer_reset_counter();
  kbc_reset_scancode();
  kbc_clear_scancode();
  mouse_reset_packet_state();

  if (vbe_init(APP_VIDEO_MODE) != 0)
    return 1;

  graphics_initialized = true;

  mouse_cursor_init();

  if (assets_load_all() != 0) {
    app_cleanup();
    return 1;
  }

  assets_loaded = true;

  if (renderer_init() != 0) {
    app_cleanup();
    return 1;
  }

  renderer_initialized = true;

  if (renderer_render_current_state() != 0) {
    app_cleanup();
    return 1;
  }

  uint8_t timer_bit_no;

  if (timer_subscribe_int(&timer_bit_no) != 0) {
    app_cleanup();
    return 1;
  }

  timer_irq_set = BIT(timer_bit_no);
  timer_subscribed = true;

  uint8_t kbd_bit_no;

  if (kbd_subscribe_int(&kbd_bit_no) != 0) {
    app_cleanup();
    return 1;
  }

  kbd_irq_set = BIT(kbd_bit_no);
  kbd_subscribed = true;

  if (serial_init(SERIAL_COM1_BASE, SERIAL_DEFAULT_BIT_RATE) == 0) {
    serial_initialized = true;

    uint8_t serial_bit_no;

    if (serial_subscribe_int(&serial_bit_no) == 0) {
      serial_irq_set = BIT(serial_bit_no);
      serial_subscribed = true;

      if (serial_enable_interrupts() == 0)
        serial_interrupts_enabled = true;
      else {
        printf("Warning: serial interrupts could not be enabled; multiplayer serial input is unavailable.\n");
        (void) serial_unsubscribe_int();
        serial_subscribed = false;
        serial_irq_set = 0;
      }
    }
    else {
      printf("Warning: COM1 IRQ could not be subscribed; multiplayer serial input is unavailable.\n");
    }
  }
  else {
    printf("Warning: COM1 could not be initialized; multiplayer serial input is unavailable.\n");
  }

  uint8_t mouse_bit_no;

  if (mouse_subscribe_int(&mouse_bit_no) != 0) {
    app_cleanup();
    return 1;
  }

  mouse_irq_set = BIT(mouse_bit_no);
  mouse_subscribed = true;

  if (mouse_enable_data_reporting_cmd() != 0) {
    app_cleanup();
    return 1;
  }

  mouse_data_reporting_enabled = true;

  return 0;
}

int app_run(void) {
  int ipc_status;
  message msg;

  while (state_get_current() != STATE_EXIT) {
    bool render_requested = false;
    int r = driver_receive(ANY, &msg, &ipc_status);

    if (r != OK)
      continue;

    if (!is_ipc_notify(ipc_status))
      continue;

    switch (_ENDPOINT_P(msg.m_source)) {
      case HARDWARE:
        if (msg.m_notify.interrupts & serial_irq_set)
          app_handle_serial_interrupt();

        if (msg.m_notify.interrupts & mouse_irq_set)
          app_handle_mouse_interrupt();

        if (msg.m_notify.interrupts & kbd_irq_set)
          app_handle_keyboard_interrupt();

        if (msg.m_notify.interrupts & timer_irq_set)
          render_requested = app_handle_timer_interrupt();

        if (render_requested)
          app_render_frame();

        break;

      default:
        break;
    }
  }

  return 0;
}

int app_cleanup(void) {
  int ret = 0;

  state_set_current(STATE_EXIT);

  printf("Application stopped. Timer interrupts: %u | Keyboard events: %u | Mouse packets: %u | Serial interrupts: %u | Rendered frames: %u | Final state: %s\n",
         timer_get_counter(),
         keyboard_events,
         mouse_packets,
         serial_interrupts,
         rendered_frames,
         state_to_string(state_get_current()));

  if (mouse_data_reporting_enabled) {
    if (mouse_disable_data_reporting() != 0)
      ret = 1;

    mouse_data_reporting_enabled = false;
  }

  if (mouse_subscribed) {
    if (mouse_unsubscribe_int() != 0)
      ret = 1;
    else {
      mouse_subscribed = false;
      mouse_irq_set = 0;
    }
  }

  if (kbd_subscribed) {
    if (kbd_unsubscribe_int() != 0)
      ret = 1;
    else {
      kbd_subscribed = false;
      kbd_irq_set = 0;
    }
  }

  if (serial_interrupts_enabled) {
    if (serial_disable_interrupts() != 0)
      ret = 1;

    serial_interrupts_enabled = false;
  }

  if (serial_subscribed) {
    if (serial_unsubscribe_int() != 0)
      ret = 1;
    else {
      serial_subscribed = false;
      serial_irq_set = 0;
    }
  }

  if (serial_initialized) {
    if (serial_shutdown() != 0)
      ret = 1;

    serial_initialized = false;
  }

  if (timer_subscribed) {
    if (timer_unsubscribe_int() != 0)
      ret = 1;
    else {
      timer_subscribed = false;
      timer_irq_set = 0;
    }
  }

  if (renderer_initialized) {
    if (renderer_shutdown() != 0)
      ret = 1;

    renderer_initialized = false;
  }

  if (assets_loaded) {
    assets_free_all();
    assets_loaded = false;
  }

  if (graphics_initialized) {
    if (vbe_exit() != 0)
      ret = 1;

    graphics_initialized = false;
  }

  frame_tick_counter = 0;

  return ret;
}
