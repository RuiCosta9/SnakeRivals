#ifndef KBC_H
#define KBC_H

#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * @file kbc.h
 * @brief Keyboard controller driver reused by the project.
 */

/** @brief Read KBC status. @param status Output status byte. @return 0 on success. */
int kbc_read_status(uint8_t *status);

/** @brief Read KBC output buffer. @param data Output byte. @return 0 on success. */
int kbc_read_outbuf(uint8_t *data);

/** @brief Write a KBC command. @param cmd Command byte. @return 0 on success. */
int kbc_write_command(uint8_t cmd);

/** @brief Write a KBC command argument. @param arg Argument byte. @return 0 on success. */
int kbc_write_argument(uint8_t arg);

/** @brief Read the KBC command byte. @param cmd_byte Output byte. @return 0 on success. */
int kbc_read_cmd_byte(uint8_t *cmd_byte);

/** @brief Write the KBC command byte. @param cmd_byte Command byte. @return 0 on success. */
int kbc_write_cmd_byte(uint8_t cmd_byte);

/** @brief Enable or disable keyboard interrupts. @param enable Desired state. @return 0 on success. */
int kbc_set_keyboard_interrupts(bool enable);

/** @brief Subscribe keyboard interrupts. @param bit_no Output interrupt bit. @return 0 on success. */
int kbd_subscribe_int(uint8_t *bit_no);

/** @brief Unsubscribe keyboard interrupts. @return 0 on success. */
int kbd_unsubscribe_int(void);

/**
 * @brief Keyboard interrupt handler.
 *
 * @note Stores scancode bytes for later state dispatch.
 */
void(kbc_ih)(void);

/** @brief Check whether the latest keyboard read had an error. @return true on error. */
bool kbc_get_error(void);

/** @brief Get the latest completed scancode. @return Scancode byte. */
uint8_t kbc_get_scancode(void);

/** @brief Reset internal scancode state. */
void kbc_reset_scancode(void);

/** @brief Process one raw scancode byte. @param byte Raw byte from KBC. */
void kbc_process_scancode_byte(uint8_t byte);

/** @brief Check whether a complete scancode is ready. @return true if ready. */
bool kbc_scancode_ready(void);

/** @brief Get completed scancode size. @return Size in bytes. */
uint8_t kbc_get_scancode_size(void);

/** @brief Copy the completed scancode bytes. @param bytes Output array of size 2. */
void kbc_get_scancode_bytes(uint8_t bytes[2]);

/** @brief Check whether the completed scancode is a make code. @return true for make. */
bool kbc_scancode_is_make(void);

/** @brief Clear the completed scancode flag. */
void kbc_clear_scancode(void);

#ifdef LAB3
unsigned long kbc_get_sys_inb_count(void);
void kbc_reset_sys_inb_count(void);
#endif

#endif
