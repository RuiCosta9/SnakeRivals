#include <lcom/lcf.h>
#include <lcom/utils.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "i8042.h"
#include "mouse.h"

#define MOUSE_HOOK_ID 2

static int hook_id_mouse = MOUSE_HOOK_ID;
static bool mouse_subscribed = false;

static uint8_t mouse_byte = 0;
static bool mouse_error = false;

static uint8_t packet_bytes[MOUSE_PACKET_BYTES];
static uint8_t packet_index = 0;
static bool packet_ready = false;

typedef enum {
  MOUSE_CMD_OK,
  MOUSE_CMD_RETRY,
  MOUSE_CMD_FAIL
} mouse_cmd_result_t;

static int mouse_read_outbuf(uint8_t *data);
static int mouse_mask_irq(bool *masked);
static int mouse_unmask_irq(bool masked);
static int mouse_wait_for_ibf_clear(void);
static mouse_cmd_result_t mouse_send_cmd_attempt(uint8_t cmd);
static int mouse_send_cmd(uint8_t cmd);
static int16_t mouse_sign_extend(bool sign_bit, uint8_t value);

int mouse_subscribe_int(uint8_t *bit_no) {
  if (bit_no == NULL) return 1;

  hook_id_mouse = MOUSE_HOOK_ID;
  *bit_no = (uint8_t) hook_id_mouse;

  if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_mouse) != 0)
    return 1;

  mouse_subscribed = true;
  return 0;
}

int mouse_unsubscribe_int(void) {
  if (!mouse_subscribed)
    return 0;

  if (sys_irqrmpolicy(&hook_id_mouse) != 0)
    return 1;

  mouse_subscribed = false;
  hook_id_mouse = MOUSE_HOOK_ID;
  return 0;
}

void (mouse_ih)(void) {
  uint8_t byte = 0;

  if (mouse_read_outbuf(&byte) != 0) {
    mouse_error = true;
    return;
  }

  mouse_byte = byte;
  mouse_error = false;
}

uint8_t mouse_get_byte(void) {
  return mouse_byte;
}

bool mouse_get_error(void) {
  return mouse_error;
}

int mouse_enable_data_reporting_cmd(void) {
  return mouse_send_cmd(MOUSE_ENABLE_DR);
}

int (mouse_disable_data_reporting)(void) {
  return mouse_send_cmd(MOUSE_DISABLE_DR);
}

void mouse_reset_packet_state(void) {
  memset(packet_bytes, 0, sizeof(packet_bytes));
  packet_index = 0;
  packet_ready = false;
  mouse_byte = 0;
  mouse_error = false;
}

void mouse_sync_packet(uint8_t byte) {
  packet_ready = false;

  if (packet_index == 0 && !(byte & FIRST_BYTE_SYNC))
    return;

  packet_bytes[packet_index] = byte;
  packet_index++;

  if (packet_index == MOUSE_PACKET_BYTES) {
    packet_ready = true;
    packet_index = 0;
  }
}

bool mouse_packet_ready(void) {
  return packet_ready;
}

int mouse_get_packet(struct packet *pp) {
  if (pp == NULL) return 1;
  if (!packet_ready) return 1;

  pp->bytes[0] = packet_bytes[0];
  pp->bytes[1] = packet_bytes[1];
  pp->bytes[2] = packet_bytes[2];

  pp->lb = (packet_bytes[0] & MOUSE_LB) != 0;
  pp->rb = (packet_bytes[0] & MOUSE_RB) != 0;
  pp->mb = (packet_bytes[0] & MOUSE_MB) != 0;

  pp->x_ov = (packet_bytes[0] & MOUSE_X_OVFL) != 0;
  pp->y_ov = (packet_bytes[0] & MOUSE_Y_OVFL) != 0;

  bool x_sign = (packet_bytes[0] & MOUSE_X_SIGN) != 0;
  bool y_sign = (packet_bytes[0] & MOUSE_Y_SIGN) != 0;

  pp->delta_x = mouse_sign_extend(x_sign, packet_bytes[1]);
  pp->delta_y = mouse_sign_extend(y_sign, packet_bytes[2]);

  packet_ready = false;
  return 0;
}

static int mouse_read_outbuf(uint8_t *data) {
  uint8_t status = 0;
  uint8_t byte = 0;

  if (data == NULL) return 1;

  if (util_sys_inb(KBC_ST_REG, &status) != 0)
    return 1;

  if (!(status & KBC_OBF))
    return 1;

  if (util_sys_inb(KBC_OUT_BUF, &byte) != 0)
    return 1;

  if (status & (KBC_PARITY_ERR | KBC_TIMEOUT_ERR))
    return 1;

  if (!(status & KBC_AUX))
    return 1;

  *data = byte;
  return 0;
}

static int mouse_mask_irq(bool *masked) {
  if (masked == NULL) return 1;

  *masked = false;

  if (!mouse_subscribed)
    return 0;

  if (sys_irqdisable(&hook_id_mouse) != 0)
    return 1;

  *masked = true;
  return 0;
}

static int mouse_unmask_irq(bool masked) {
  if (!masked)
    return 0;

  if (sys_irqenable(&hook_id_mouse) != 0)
    return 1;

  return 0;
}

static int mouse_wait_for_ibf_clear(void) {
  uint8_t status = 0;

  for (int tries = 0; tries < MAX_TRIES; tries++) {
    if (util_sys_inb(KBC_ST_REG, &status) != 0)
      return 1;

    if (!(status & KBC_IBF))
      return 0;

    tickdelay(micros_to_ticks(DELAY_US));
  }

  return 1;
}

static mouse_cmd_result_t mouse_send_cmd_attempt(uint8_t cmd) {
  uint8_t response = 0;

  if (mouse_wait_for_ibf_clear() != 0)
    return MOUSE_CMD_FAIL;

  if (sys_outb(KBC_CMD_REG, WRITE_BYTE_TO_MOUSE) != 0)
    return MOUSE_CMD_FAIL;

  if (mouse_wait_for_ibf_clear() != 0)
    return MOUSE_CMD_FAIL;

  if (sys_outb(KBC_IN_BUF, cmd) != 0)
    return MOUSE_CMD_FAIL;

  for (int tries = 0; tries < MAX_TRIES; tries++) {
    if (mouse_read_outbuf(&response) == 0) {
      switch (response) {
        case MOUSE_ACK:
          return MOUSE_CMD_OK;

        case MOUSE_NACK:
        case MOUSE_ERROR:
          return MOUSE_CMD_RETRY;

        default:
          break;
      }
    }

    tickdelay(micros_to_ticks(DELAY_US));
  }

  return MOUSE_CMD_RETRY;
}

static int mouse_send_cmd(uint8_t cmd) {
  bool irq_masked = false;
  int ret = 1;

  if (mouse_mask_irq(&irq_masked) != 0)
    return 1;

  for (int attempt = 0; attempt < MAX_TRIES; attempt++) {
    mouse_cmd_result_t result = mouse_send_cmd_attempt(cmd);

    if (result == MOUSE_CMD_OK) {
      ret = 0;
      break;
    }

    if (result == MOUSE_CMD_FAIL)
      break;
  }

  if (mouse_unmask_irq(irq_masked) != 0)
    return 1;

  return ret;
}

static int16_t mouse_sign_extend(bool sign_bit, uint8_t value) {
  if (sign_bit)
    return (int16_t)((int16_t)value - 256);

  return (int16_t)value;
}
