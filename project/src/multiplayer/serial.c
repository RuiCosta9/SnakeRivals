/**
 * @file serial.c
 * @brief UART driver implementation for multiplayer communication.
 */

#include "serial.h"

#include <lcom/lcf.h>
#include <lcom/utils.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "serial_queue.h"

#define SERIAL_RBR_OFFSET 0
#define SERIAL_THR_OFFSET 0
#define SERIAL_DLL_OFFSET 0
#define SERIAL_IER_OFFSET 1
#define SERIAL_DLM_OFFSET 1
#define SERIAL_IIR_OFFSET 2
#define SERIAL_FCR_OFFSET 2
#define SERIAL_LCR_OFFSET 3
#define SERIAL_MCR_OFFSET 4
#define SERIAL_LSR_OFFSET 5

#define SERIAL_UART_BASE_RATE 115200

#define SERIAL_IER_RECEIVED_DATA_AVAILABLE BIT(0)
#define SERIAL_IER_TRANSMITTER_EMPTY       BIT(1)
#define SERIAL_IER_RECEIVER_LINE_STATUS    BIT(2)

#define SERIAL_IIR_NO_INTERRUPT_PENDING BIT(0)
#define SERIAL_IIR_INTERRUPT_ID_MASK    (BIT(1) | BIT(2) | BIT(3))
#define SERIAL_IIR_TX_EMPTY             BIT(1)
#define SERIAL_IIR_RECEIVED_DATA        BIT(2)
#define SERIAL_IIR_LINE_STATUS          (BIT(1) | BIT(2))
#define SERIAL_IIR_CHARACTER_TIMEOUT    (BIT(2) | BIT(3))

#define SERIAL_FCR_ENABLE_FIFO BIT(0)
#define SERIAL_FCR_CLEAR_RX    BIT(1)
#define SERIAL_FCR_CLEAR_TX    BIT(2)

#define SERIAL_LCR_8_BITS       (BIT(0) | BIT(1))
#define SERIAL_LCR_1_STOP_BIT   0
#define SERIAL_LCR_NO_PARITY    0
#define SERIAL_LCR_DLAB         BIT(7)
#define SERIAL_LCR_8N1          (SERIAL_LCR_8_BITS | SERIAL_LCR_1_STOP_BIT | SERIAL_LCR_NO_PARITY)

#define SERIAL_MCR_DTR  BIT(0)
#define SERIAL_MCR_RTS  BIT(1)
#define SERIAL_MCR_OUT2 BIT(3)

#define SERIAL_LSR_DATA_READY     BIT(0)
#define SERIAL_LSR_OVERRUN_ERROR  BIT(1)
#define SERIAL_LSR_PARITY_ERROR   BIT(2)
#define SERIAL_LSR_FRAMING_ERROR  BIT(3)
#define SERIAL_LSR_THR_EMPTY      BIT(5)

#define SERIAL_LSR_ERROR_MASK (SERIAL_LSR_OVERRUN_ERROR | SERIAL_LSR_PARITY_ERROR | SERIAL_LSR_FRAMING_ERROR)

#define SERIAL_TX_RETRIES 10000
#define SERIAL_IH_MAX_ITERATIONS 32
#define SERIAL_RX_DRAIN_LIMIT 64

static uint16_t serial_base_addr = SERIAL_COM1_BASE;
static int hook_id_serial = SERIAL_COM1_IRQ;

static bool serial_initialized = false;
static bool serial_subscribed = false;
static bool serial_error = false;
static uint8_t serial_last_lsr = 0;

static serial_queue_t serial_rx_queue;

static uint16_t serial_reg(uint8_t offset) {
  return serial_base_addr + offset;
}

static int serial_read_reg(uint8_t offset, uint8_t *value) {
  if (value == NULL)
    return 1;

  return util_sys_inb(serial_reg(offset), value);
}

static int serial_write_reg(uint8_t offset, uint8_t value) {
  return sys_outb(serial_reg(offset), value);
}

static void serial_record_lsr(uint8_t lsr) {
  serial_last_lsr = lsr;

  if (lsr & SERIAL_LSR_ERROR_MASK)
    serial_error = true;
}

static int serial_set_bit_rate(uint32_t bit_rate) {
  if (bit_rate == 0 || SERIAL_UART_BASE_RATE % bit_rate != 0)
    return 1;

  uint32_t divisor32 = SERIAL_UART_BASE_RATE / bit_rate;

  if (divisor32 == 0 || divisor32 > UINT16_MAX)
    return 1;

  uint16_t divisor = (uint16_t) divisor32;
  uint8_t lcr = 0;

  if (serial_read_reg(SERIAL_LCR_OFFSET, &lcr) != 0)
    return 1;

  if (serial_write_reg(SERIAL_LCR_OFFSET, lcr | SERIAL_LCR_DLAB) != 0)
    return 1;

  if (serial_write_reg(SERIAL_DLL_OFFSET, (uint8_t) (divisor & 0xFF)) != 0)
    return 1;

  if (serial_write_reg(SERIAL_DLM_OFFSET, (uint8_t) ((divisor >> 8) & 0xFF)) != 0)
    return 1;

  return serial_write_reg(SERIAL_LCR_OFFSET, SERIAL_LCR_8N1);
}

static int serial_drain_received_data(void) {
  for (uint16_t i = 0; i < SERIAL_RX_DRAIN_LIMIT; i++) {
    uint8_t lsr = 0;

    if (serial_read_reg(SERIAL_LSR_OFFSET, &lsr) != 0) {
      serial_error = true;
      return 1;
    }

    serial_record_lsr(lsr);

    if (!(lsr & SERIAL_LSR_DATA_READY))
      return 0;

    uint8_t byte = 0;

    if (serial_read_reg(SERIAL_RBR_OFFSET, &byte) != 0) {
      serial_error = true;
      return 1;
    }

    if (serial_queue_push(&serial_rx_queue, byte) != 0)
      serial_error = true;
  }

  return 0;
}

int serial_init(uint16_t base_addr, uint32_t bit_rate) {
  serial_base_addr = base_addr;
  serial_error = false;
  serial_last_lsr = 0;
  serial_queue_init(&serial_rx_queue);

  if (serial_disable_interrupts() != 0)
    return 1;

  if (serial_set_bit_rate(bit_rate) != 0)
    return 1;

  if (serial_write_reg(SERIAL_FCR_OFFSET, SERIAL_FCR_ENABLE_FIFO | SERIAL_FCR_CLEAR_RX | SERIAL_FCR_CLEAR_TX) != 0)
    return 1;

  if (serial_write_reg(SERIAL_MCR_OFFSET, SERIAL_MCR_DTR | SERIAL_MCR_RTS | SERIAL_MCR_OUT2) != 0)
    return 1;

  if (serial_drain_received_data() != 0)
    return 1;

  serial_error = false;
  serial_initialized = true;

  return 0;
}

int serial_shutdown(void) {
  int ret = 0;

  if (serial_disable_interrupts() != 0)
    ret = 1;

  serial_queue_clear(&serial_rx_queue);
  serial_initialized = false;

  return ret;
}

int serial_subscribe_int(uint8_t *bit_no) {
  if (bit_no == NULL)
    return 1;

  hook_id_serial = SERIAL_COM1_IRQ;
  *bit_no = (uint8_t) hook_id_serial;

  if (sys_irqsetpolicy(SERIAL_COM1_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_serial) != OK)
    return 1;

  serial_subscribed = true;
  return 0;
}

int serial_unsubscribe_int(void) {
  if (!serial_subscribed)
    return 0;

  if (serial_disable_interrupts() != 0)
    return 1;

  if (sys_irqrmpolicy(&hook_id_serial) != OK)
    return 1;

  serial_subscribed = false;
  hook_id_serial = SERIAL_COM1_IRQ;

  return 0;
}

int serial_enable_interrupts(void) {
  if (!serial_initialized)
    return 1;

  return serial_write_reg(SERIAL_IER_OFFSET, SERIAL_IER_RECEIVED_DATA_AVAILABLE | SERIAL_IER_RECEIVER_LINE_STATUS);
}

int serial_disable_interrupts(void) {
  return serial_write_reg(SERIAL_IER_OFFSET, 0);
}

void serial_ih(void) {
  if (!serial_initialized)
    return;

  for (uint8_t i = 0; i < SERIAL_IH_MAX_ITERATIONS; i++) {
    uint8_t iir = 0;

    if (serial_read_reg(SERIAL_IIR_OFFSET, &iir) != 0) {
      serial_error = true;
      return;
    }

    if (iir & SERIAL_IIR_NO_INTERRUPT_PENDING)
      return;

    switch (iir & SERIAL_IIR_INTERRUPT_ID_MASK) {
      case SERIAL_IIR_LINE_STATUS: {
        uint8_t lsr = 0;

        if (serial_read_reg(SERIAL_LSR_OFFSET, &lsr) != 0) {
          serial_error = true;
          return;
        }

        serial_record_lsr(lsr);
        break;
      }

      case SERIAL_IIR_RECEIVED_DATA:
      case SERIAL_IIR_CHARACTER_TIMEOUT:
        (void) serial_drain_received_data();
        break;

      case SERIAL_IIR_TX_EMPTY:
        /*
         * TX is currently polling-based. If this driver grows a TX queue,
         * this is where queued bytes should be written after the first manual
         * byte starts transmission from an idle UART.
         */
        break;

      default:
        return;
    }
  }
}

int serial_send_byte(uint8_t byte) {
  if (!serial_initialized)
    return 1;

  for (uint16_t tries = 0; tries < SERIAL_TX_RETRIES; tries++) {
    uint8_t lsr = 0;

    if (serial_read_reg(SERIAL_LSR_OFFSET, &lsr) != 0) {
      serial_error = true;
      return 1;
    }

    serial_record_lsr(lsr);

    if (lsr & SERIAL_LSR_THR_EMPTY)
      return serial_write_reg(SERIAL_THR_OFFSET, byte);
  }

  serial_error = true;
  return 1;
}

int serial_send_bytes(const uint8_t *bytes, uint16_t size) {
  if (bytes == NULL && size > 0)
    return 1;

  for (uint16_t i = 0; i < size; i++) {
    if (serial_send_byte(bytes[i]) != 0)
      return 1;
  }

  return 0;
}

bool serial_rx_available(void) {
  return !serial_queue_is_empty(&serial_rx_queue);
}

int serial_receive_byte(uint8_t *byte) {
  return serial_queue_pop(&serial_rx_queue, byte);
}

bool serial_has_error(void) {
  return serial_error;
}

uint8_t serial_get_last_lsr(void) {
  return serial_last_lsr;
}
