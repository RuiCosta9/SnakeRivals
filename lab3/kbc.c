#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>

#include "i8042.h"
#include "kbc.h"

#define KBD_HOOK_ID 1
#define BREAK_CODE_BIT BIT(7)

static bool kbc_error = false;

static int hook_id_kbd = KBD_HOOK_ID;
static bool kbd_subscribed = false;

static uint8_t last_scancode_byte = 0;

static uint8_t scancode_bytes[2] = {0, 0};
static uint8_t scancode_size = 0;
static bool scancode_complete = false;
static bool scancode_make = false;

#ifdef LAB3
static unsigned long sys_inb_count = 0;
#endif

static int kbc_sys_inb(int port, uint8_t *value) {
  if (value == NULL)
    return 1;

  uint32_t val = 0;

  if (sys_inb(port, &val) != OK)
    return 1;

  *value = (uint8_t) val;

#ifdef LAB3
  sys_inb_count++;
#endif

  return 0;
}

static int kbc_read_output_data(uint8_t *data) {
  if (data == NULL)
    return 1;

  uint8_t status;

  if (kbc_read_status(&status) != 0)
    return 1;

  if ((status & KBC_OBF) == 0)
    return 1;

  if (kbc_sys_inb(KBC_OUT_BUF, data) != 0)
    return 1;

  if (status & (KBC_PARITY | KBC_TIMEOUT))
    return 1;

  if (status & KBC_AUX)
    return 1;

  return 0;
}

static int kbc_write_to_port(uint8_t port, uint8_t value) {
  uint8_t status;

  for (int tries = 0; tries < RETRIES; tries++) {
    if (kbc_read_status(&status) != 0)
      return 1;

    if ((status & KBC_IBF) == 0) {
      if (sys_outb(port, value) != OK)
        return 1;

      return 0;
    }

    tickdelay(micros_to_ticks(DELAY_US));
  }

  return 1;
}

bool kbc_get_error(void) {
  return kbc_error;
}

uint8_t kbc_get_scancode(void) {
  return last_scancode_byte;
}

void kbc_reset_scancode(void) {
  last_scancode_byte = 0;
}

#ifdef LAB3
unsigned long kbc_get_sys_inb_count(void) {
  return sys_inb_count;
}

void kbc_reset_sys_inb_count(void) {
  sys_inb_count = 0;
}
#endif

int kbc_read_status(uint8_t *status) {
  if (status == NULL)
    return 1;

  return kbc_sys_inb(KBC_ST_REG, status);
}

int kbc_read_outbuf(uint8_t *data) {
  return kbc_read_output_data(data);
}

int kbc_write_command(uint8_t cmd) {
  return kbc_write_to_port(KBC_CMD_REG, cmd);
}

int kbc_write_argument(uint8_t arg) {
  return kbc_write_to_port(KBC_IN_BUF, arg);
}

int kbc_read_cmd_byte(uint8_t *cmd_byte) {
  if (cmd_byte == NULL)
    return 1;

  if (kbc_write_command(READ_CMD_BYTE) != 0)
    return 1;

  for (int tries = 0; tries < RETRIES; tries++) {
    if (kbc_read_output_data(cmd_byte) == 0)
      return 0;

    tickdelay(micros_to_ticks(DELAY_US));
  }

  return 1;
}

int kbc_write_cmd_byte(uint8_t cmd_byte) {
  if (kbc_write_command(WRITE_CMD_BYTE) != 0)
    return 1;

  if (kbc_write_argument(cmd_byte) != 0)
    return 1;

  return 0;
}

int kbc_set_keyboard_interrupts(bool enable) {
  uint8_t cmd_byte;

  if (kbc_read_cmd_byte(&cmd_byte) != 0)
    return 1;

  if (enable)
    cmd_byte |= KBC_INT;
  else
    cmd_byte &= ~KBC_INT;

  return kbc_write_cmd_byte(cmd_byte);
}

int kbd_subscribe_int(uint8_t *bit_no) {
  if (bit_no == NULL)
    return 1;

  hook_id_kbd = KBD_HOOK_ID;
  *bit_no = (uint8_t) hook_id_kbd;

  if (sys_irqsetpolicy(KBD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_kbd) != OK)
    return 1;

  kbd_subscribed = true;
  return 0;
}

int kbd_unsubscribe_int(void) {
  if (!kbd_subscribed)
    return 0;

  if (sys_irqrmpolicy(&hook_id_kbd) != OK)
    return 1;

  kbd_subscribed = false;
  hook_id_kbd = KBD_HOOK_ID;

  return 0;
}

void kbc_clear_scancode(void) {
  scancode_bytes[0] = 0;
  scancode_bytes[1] = 0;
  scancode_size = 0;
  scancode_complete = false;
  scancode_make = false;
}

void kbc_process_scancode_byte(uint8_t byte) {
  scancode_complete = false;

  if (byte == TWO_BYTE_CODE) {
    scancode_bytes[0] = byte;
    scancode_bytes[1] = 0;
    scancode_size = 1;
    return;
  }

  if (scancode_size == 1 && scancode_bytes[0] == TWO_BYTE_CODE) {
    scancode_bytes[1] = byte;
    scancode_size = 2;
  }
  else {
    scancode_bytes[0] = byte;
    scancode_bytes[1] = 0;
    scancode_size = 1;
  }

  scancode_make = !(byte & BREAK_CODE_BIT);
  scancode_complete = true;
}

bool kbc_scancode_ready(void) {
  return scancode_complete;
}

uint8_t kbc_get_scancode_size(void) {
  return scancode_size;
}

void kbc_get_scancode_bytes(uint8_t bytes[2]) {
  if (bytes == NULL)
    return;

  bytes[0] = scancode_bytes[0];
  bytes[1] = scancode_bytes[1];
}

bool kbc_scancode_is_make(void) {
  return scancode_make;
}

void(kbc_ih)(void) {
  uint8_t data;

  kbc_error = false;

  if (kbc_read_output_data(&data) != 0) {
    kbc_error = true;
    return;
  }

  last_scancode_byte = data;
  kbc_process_scancode_byte(data);
}
