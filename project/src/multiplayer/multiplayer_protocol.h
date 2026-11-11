#ifndef MULTIPLAYER_PROTOCOL_H
#define MULTIPLAYER_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#include "powerup.h"
#include "snake.h"

/**
 * @file multiplayer_protocol.h
 * @brief Serial message protocol used by multiplayer sessions.
 *
 * Provides frame parsing/encoding and helpers for each message type. The
 * session layer decides when to send messages and how to apply decoded data.
 */

#define MP_START_BYTE 0x7E
#define MP_END_BYTE   0x7F
#define MP_MAX_PAYLOAD_SIZE 255
#define MP_SNAPSHOT_PLAYER_COUNT 2
#define MP_SNAPSHOT_EFFECT_SLOTS 2
#define MP_INPUT_FLAG_ACTIVATE_POWERUP 0x01

/**
 * @brief Message identifiers transported over the serial protocol.
 */
typedef enum {
  MP_MSG_HELLO = 0x01,        /**< Initial handshake message. */
  MP_MSG_ACK = 0x02,          /**< Acknowledgement/response message. */
  MP_MSG_INPUT = 0x03,        /**< Client input: direction and powerup activation. */
  MP_MSG_SNAPSHOT = 0x04,     /**< Host authoritative game-state snapshot. */
  MP_MSG_PING = 0x05,         /**< Lightweight connectivity check. */
  MP_MSG_JOIN_REQUEST = 0x06, /**< Client room-code join request. */
  MP_MSG_JOIN_ACCEPT = 0x07,  /**< Host accepted the join request. */
  MP_MSG_JOIN_REJECT = 0x08,  /**< Host rejected the join request. */
  MP_MSG_START = 0x09,        /**< Host starts the match in a selected mode. */
  MP_MSG_SKIN_SELECT = 0x0A   /**< Lobby skin selection request. */
} mp_message_type_t;

/**
 * @brief Decoded protocol message before/after frame encoding.
 */
typedef struct {
  mp_message_type_t type;              /**< Message type. */
  uint8_t length;                      /**< Payload size in bytes. */
  uint8_t payload[MP_MAX_PAYLOAD_SIZE];/**< Raw message payload. */
} mp_message_t;

/**
 * @brief Snapshot data for one player.
 */
typedef struct {
  snake_t snake;                                      /**< Rebuilt snake state. */
  uint32_t score;                                     /**< Player score. */
  bool alive;                                         /**< Whether the player is alive. */
  bool food_active;                                   /**< Whether this player's food is visible. */
  position_t food_position;                           /**< Food position. */
  bool powerup_active;                                /**< Whether this player's powerup is visible. */
  position_t powerup_position;                        /**< Powerup position. */
  powerup_type_t powerup_type;                        /**< Visible powerup type. */
  bool held_powerup_active;                           /**< Whether inventory has a held powerup. */
  powerup_type_t held_powerup;                        /**< Held powerup type. */
  bool queued_powerup_active;                         /**< Whether inventory has a queued powerup. */
  powerup_type_t queued_powerup;                      /**< Queued powerup type. */
  bool shield_active;                                 /**< Whether a shield is armed. */
  uint8_t effect_count;                               /**< Number of active effects encoded below. */
  powerup_type_t effect_types[MP_SNAPSHOT_EFFECT_SLOTS]; /**< Active effect types. */
  uint16_t effect_ticks[MP_SNAPSHOT_EFFECT_SLOTS];    /**< Remaining effect durations. */
} mp_player_snapshot_t;

/**
 * @brief Authoritative game-state snapshot sent from host to client.
 */
typedef struct {
  uint16_t tick;                                      /**< Host simulation tick. */
  uint8_t mode;                                       /**< Multiplayer game mode. */
  uint8_t phase;                                      /**< Multiplayer match phase. */
  uint8_t winner;                                     /**< Winner identifier when finished. */
  uint16_t countdown_ticks;                           /**< Remaining countdown ticks. */
  uint8_t grid_width;                                 /**< Active grid width. */
  uint8_t grid_height;                                /**< Active grid height. */
  uint8_t player_skins[MP_SNAPSHOT_PLAYER_COUNT];     /**< Skin index for each player. */
  mp_player_snapshot_t players[MP_SNAPSHOT_PLAYER_COUNT]; /**< Player snapshots. */
} mp_snapshot_t;

/**
 * @brief Incremental byte-parser result.
 */
typedef enum {
  MP_PARSE_IN_PROGRESS, /**< More bytes are needed. */
  MP_PARSE_COMPLETE,    /**< A complete valid message was decoded. */
  MP_PARSE_ERROR        /**< Frame or checksum error. */
} mp_parse_result_t;

/**
 * @brief Reset the incremental protocol parser.
 */
void mp_protocol_parser_reset(void);

/**
 * @brief Feed one byte to the protocol parser.
 *
 * @param byte Byte received from the serial port.
 * @param message Output message filled when parsing completes.
 * @return Parser status after consuming the byte.
 */
mp_parse_result_t mp_protocol_parse_byte(uint8_t byte, mp_message_t *message);

/**
 * @brief Encode a protocol message into a framed byte sequence.
 *
 * @param message Message to encode.
 * @param out Output buffer.
 * @param out_capacity Size of @p out in bytes.
 * @param out_size Output frame size.
 * @return 0 on success, non-zero on invalid input or insufficient capacity.
 */
int mp_protocol_encode_message(const mp_message_t *message, uint8_t *out, uint16_t out_capacity, uint16_t *out_size);

/**
 * @brief Build a HELLO message.
 *
 * @param message Output message.
 * @return 0 on success, non-zero on invalid input.
 */
int mp_protocol_make_hello(mp_message_t *message);

/**
 * @brief Build an ACK message.
 *
 * @param message Output message.
 * @return 0 on success, non-zero on invalid input.
 */
int mp_protocol_make_ack(mp_message_t *message);

/**
 * @brief Build an INPUT message.
 *
 * @param player_id Player identifier.
 * @param direction Direction requested by the player.
 * @param activate_powerup Whether the player requested powerup activation.
 * @param message Output message.
 * @return 0 on success, non-zero on invalid input.
 */
int mp_protocol_make_input(uint8_t player_id, direction_t direction, bool activate_powerup, mp_message_t *message);

/**
 * @brief Decode an INPUT message.
 *
 * @param message Message to decode.
 * @param player_id Output player identifier.
 * @param direction Output direction.
 * @param activate_powerup Output activation flag.
 * @return 0 on success, non-zero on invalid message.
 */
int mp_protocol_read_input(const mp_message_t *message, uint8_t *player_id, direction_t *direction, bool *activate_powerup);

/**
 * @brief Build a SNAPSHOT message.
 *
 * @param snapshot Snapshot to encode.
 * @param message Output message.
 * @return 0 on success, non-zero if the snapshot cannot be encoded.
 */
int mp_protocol_make_snapshot(const mp_snapshot_t *snapshot, mp_message_t *message);

/**
 * @brief Decode a SNAPSHOT message.
 *
 * @param message Message to decode.
 * @param snapshot Output snapshot.
 * @return 0 on success, non-zero on invalid message.
 */
int mp_protocol_read_snapshot(const mp_message_t *message, mp_snapshot_t *snapshot);

/**
 * @brief Build a PING message.
 *
 * @param message Output message.
 * @return 0 on success, non-zero on invalid input.
 */
int mp_protocol_make_ping(mp_message_t *message);

/**
 * @brief Build a JOIN_REQUEST message.
 *
 * @param code Room code requested by the client.
 * @param message Output message.
 * @return 0 on success, non-zero if code/input is invalid.
 */
int mp_protocol_make_join_request(uint16_t code, mp_message_t *message);

/**
 * @brief Read a room code from a JOIN_REQUEST message.
 *
 * @param message Message to decode.
 * @param code Output room code.
 * @return 0 on success, non-zero on invalid message.
 */
int mp_protocol_read_join_code(const mp_message_t *message, uint16_t *code);

/**
 * @brief Build a JOIN_ACCEPT message.
 *
 * @param message Output message.
 * @return 0 on success, non-zero on invalid input.
 */
int mp_protocol_make_join_accept(mp_message_t *message);

/**
 * @brief Build a JOIN_REJECT message.
 *
 * @param message Output message.
 * @return 0 on success, non-zero on invalid input.
 */
int mp_protocol_make_join_reject(mp_message_t *message);

/**
 * @brief Build a START message.
 *
 * @param mode Multiplayer mode selected by the host.
 * @param message Output message.
 * @return 0 on success, non-zero on invalid input.
 */
int mp_protocol_make_start(uint8_t mode, mp_message_t *message);

/**
 * @brief Read the selected mode from a START message.
 *
 * @param message Message to decode.
 * @param mode Output mode.
 * @return 0 on success, non-zero on invalid input.
 */
int mp_protocol_read_start_mode(const mp_message_t *message, uint8_t *mode);

/**
 * @brief Build a SKIN_SELECT message.
 *
 * @param player_id Player whose skin is being selected.
 * @param skin Requested skin index.
 * @param message Output message.
 * @return 0 on success, non-zero on invalid input.
 */
int mp_protocol_make_skin_select(uint8_t player_id, uint8_t skin, mp_message_t *message);

/**
 * @brief Decode a SKIN_SELECT message.
 *
 * @param message Message to decode.
 * @param player_id Output player identifier.
 * @param skin Output skin index.
 * @return 0 on success, non-zero on invalid message.
 */
int mp_protocol_read_skin_select(const mp_message_t *message, uint8_t *player_id, uint8_t *skin);

#endif
