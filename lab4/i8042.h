#ifndef I8042_H
#define I8042_H

#include <lcom/lcf.h>

/* KBC ports */
#define KBC_ST_REG              0x64
#define KBC_CMD_REG             0x64
#define KBC_OUT_BUF             0x60
#define KBC_IN_BUF              0x60

/* Status register bits */
#define KBC_OBF                 BIT(0)
#define KBC_IBF                 BIT(1)
#define KBC_AUX                 BIT(5)
#define KBC_TIMEOUT_ERR         BIT(6)
#define KBC_PARITY_ERR          BIT(7)

/* Mouse IRQ */
#define MOUSE_IRQ               12

/* KBC command to forward byte to mouse */
#define WRITE_BYTE_TO_MOUSE     0xD4

/* Mouse commands */
#define MOUSE_ENABLE_DR         0xF4
#define MOUSE_DISABLE_DR        0xF5

/* Mouse responses */
#define MOUSE_ACK               0xFA
#define MOUSE_NACK              0xFE
#define MOUSE_ERROR             0xFC

/* Packet */
#define MOUSE_PACKET_BYTES      3
#define FIRST_BYTE_SYNC         BIT(3)

/* First packet byte bits */
#define MOUSE_LB                BIT(0)
#define MOUSE_RB                BIT(1)
#define MOUSE_MB                BIT(2)
#define MOUSE_X_SIGN            BIT(4)
#define MOUSE_Y_SIGN            BIT(5)
#define MOUSE_X_OVFL            BIT(6)
#define MOUSE_Y_OVFL            BIT(7)

/* Retries / delay */
#define DELAY_US                20000
#define MAX_TRIES               10

#endif
