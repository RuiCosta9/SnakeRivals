#include <lcom/lcf.h>

#include <stdint.h>

#include "i8254.h"
#include "timer.h"

#define TIMER_HOOK_ID 0

static int hook_id_timer = TIMER_HOOK_ID;
static unsigned int timer_counter = 0;

static int timer_get_port(uint8_t timer, uint8_t *port) {
  if (port == NULL)
    return 1;

  switch (timer) {
    case 0:
      *port = TIMER_0;
      return 0;
    case 1:
      *port = TIMER_1;
      return 0;
    case 2:
      *port = TIMER_2;
      return 0;
    default:
      return 1;
  }
}

static int timer_get_selection(uint8_t timer, uint8_t *selection) {
  if (selection == NULL)
    return 1;

  switch (timer) {
    case 0:
      *selection = TIMER_SEL0;
      return 0;
    case 1:
      *selection = TIMER_SEL1;
      return 0;
    case 2:
      *selection = TIMER_SEL2;
      return 0;
    default:
      return 1;
  }
}

int(timer_set_frequency)(uint8_t timer, uint32_t freq) {
  if (timer > 2 || freq == 0 || freq > TIMER_FREQ)
    return 1;

  uint32_t divisor32 = TIMER_FREQ / freq;

  if (divisor32 == 0 || divisor32 > UINT16_MAX)
    return 1;

  uint16_t divisor = (uint16_t) divisor32;

  uint8_t st;
  if (timer_get_conf(timer, &st) != 0)
    return 1;

  uint8_t timer_port;
  uint8_t timer_selection;

  if (timer_get_port(timer, &timer_port) != 0)
    return 1;

  if (timer_get_selection(timer, &timer_selection) != 0)
    return 1;

  uint8_t control_word = timer_selection | TIMER_LSB_MSB | (st & 0x0F);

  if (sys_outb(TIMER_CTRL, control_word) != OK)
    return 1;

  uint8_t lsb;
  uint8_t msb;

  if (util_get_LSB(divisor, &lsb) != OK)
    return 1;

  if (util_get_MSB(divisor, &msb) != OK)
    return 1;

  if (sys_outb(timer_port, lsb) != OK)
    return 1;

  if (sys_outb(timer_port, msb) != OK)
    return 1;

  return 0;
}

int(timer_subscribe_int)(uint8_t *bit_no) {
  if (bit_no == NULL)
    return 1;

  hook_id_timer = TIMER_HOOK_ID;
  *bit_no = (uint8_t) hook_id_timer;

  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id_timer) != OK)
    return 1;

  return 0;
}

int(timer_unsubscribe_int)(void) {
  if (sys_irqrmpolicy(&hook_id_timer) != OK)
    return 1;

  hook_id_timer = TIMER_HOOK_ID;
  return 0;
}

void(timer_int_handler)(void) {
  timer_counter++;
}

void timer_reset_counter(void) {
  timer_counter = 0;
}

unsigned int timer_get_counter(void) {
  return timer_counter;
}

int(timer_get_conf)(uint8_t timer, uint8_t *st) {
  if (st == NULL || timer > 2)
    return 1;

  uint8_t command = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);

  if (sys_outb(TIMER_CTRL, command) != OK)
    return 1;

  uint8_t timer_port;

  if (timer_get_port(timer, &timer_port) != 0)
    return 1;

  return util_sys_inb(timer_port, st);
}

int(timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {
  union timer_status_field_val value;

  switch (field) {
    case tsf_all:
      value.byte = st;
      break;

    case tsf_initial: {
      uint8_t init_mode = (st >> 4) & 0x03;

      switch (init_mode) {
        case 0:
          value.in_mode = INVAL_val;
          break;
        case 1:
          value.in_mode = LSB_only;
          break;
        case 2:
          value.in_mode = MSB_only;
          break;
        case 3:
          value.in_mode = MSB_after_LSB;
          break;
        default:
          return 1;
      }

      break;
    }

    case tsf_mode: {
      uint8_t mode = (st >> 1) & 0x07;

      if (mode == 6)
        mode = 2;
      else if (mode == 7)
        mode = 3;

      value.count_mode = mode;
      break;
    }

    case tsf_base:
      value.bcd = (st & TIMER_BCD) != 0;
      break;

    default:
      return 1;
  }

  return timer_print_config(timer, field, value);
}
