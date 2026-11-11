#ifndef MOUSE_H
#define MOUSE_H

#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @file mouse.h
 * @brief PS/2 mouse driver reused by the project.
 */

/** @brief Subscribe mouse interrupts. @param bit_no Output interrupt bit. @return 0 on success. */
int mouse_subscribe_int(uint8_t *bit_no);

/** @brief Unsubscribe mouse interrupts. @return 0 on success. */
int mouse_unsubscribe_int(void);

/**
 * @brief Mouse interrupt handler.
 *
 * @note Stores the latest byte and error state.
 */
void (mouse_ih)(void);

/** @brief Get the latest mouse byte. @return Raw byte. */
uint8_t mouse_get_byte(void);

/** @brief Check whether the latest mouse read had an error. @return true on error. */
bool mouse_get_error(void);

/** @brief Enable mouse data reporting through the KBC command path. @return 0 on success. */
int mouse_enable_data_reporting_cmd(void);

/** @brief Disable mouse data reporting. @return 0 on success. */
int (mouse_disable_data_reporting)(void);

/** @brief Feed one byte into the packet synchronizer. @param byte Raw mouse byte. */
void mouse_sync_packet(uint8_t byte);

/** @brief Reset packet synchronizer state. */
void mouse_reset_packet_state(void);

/** @brief Check whether a complete packet is available. @return true if ready. */
bool mouse_packet_ready(void);

/** @brief Copy the latest completed packet. @param pp Output packet. @return 0 on success. */
int mouse_get_packet(struct packet *pp);

#endif
