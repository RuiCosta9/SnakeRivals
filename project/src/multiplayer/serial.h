#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file serial.h
 * @brief UART/COM1 driver interface used by multiplayer.
 *
 * Wraps serial-port initialization, interrupt subscription, byte transmission
 * and receive-buffer access.
 */

#define SERIAL_COM1_BASE 0x3F8
#define SERIAL_COM1_IRQ  4
#define SERIAL_DEFAULT_BIT_RATE 115200

/**
 * @brief Initialize a serial port.
 *
 * @param base_addr UART base I/O address.
 * @param bit_rate Desired bit rate.
 * @return 0 on success, non-zero on failure.
 */
int serial_init(uint16_t base_addr, uint32_t bit_rate);

/**
 * @brief Shutdown the current serial port.
 *
 * @return 0 on success, non-zero on failure.
 */
int serial_shutdown(void);

/**
 * @brief Subscribe the serial interrupt line.
 *
 * @param bit_no Output interrupt bit number.
 * @return 0 on success, non-zero on failure.
 */
int serial_subscribe_int(uint8_t *bit_no);

/**
 * @brief Unsubscribe the serial interrupt line.
 *
 * @return 0 on success, non-zero on failure.
 */
int serial_unsubscribe_int(void);

/**
 * @brief Enable UART receive/transmit interrupts used by the project.
 *
 * @return 0 on success, non-zero on failure.
 */
int serial_enable_interrupts(void);

/**
 * @brief Disable UART interrupts.
 *
 * @return 0 on success, non-zero on failure.
 */
int serial_disable_interrupts(void);

/**
 * @brief Serial interrupt handler.
 *
 * @note Reads hardware status and queues received bytes.
 */
void serial_ih(void);

/**
 * @brief Send one byte through the serial port.
 *
 * @param byte Byte to transmit.
 * @return 0 on success, non-zero on failure.
 */
int serial_send_byte(uint8_t byte);

/**
 * @brief Send a byte buffer through the serial port.
 *
 * @param bytes Buffer to transmit.
 * @param size Number of bytes.
 * @return 0 on success, non-zero on failure.
 */
int serial_send_bytes(const uint8_t *bytes, uint16_t size);

/**
 * @brief Check whether a received byte is buffered.
 *
 * @return true if serial_receive_byte() can return a byte.
 */
bool serial_rx_available(void);

/**
 * @brief Pop one received byte from the serial buffer.
 *
 * @param byte Output byte.
 * @return 0 on success, non-zero when no byte is available or input is invalid.
 */
int serial_receive_byte(uint8_t *byte);

/**
 * @brief Check whether the driver observed a UART error.
 *
 * @return true if an error was detected.
 */
bool serial_has_error(void);

/**
 * @brief Get the latest line-status register value.
 *
 * @return Last LSR byte read by the driver.
 */
uint8_t serial_get_last_lsr(void);

#endif
