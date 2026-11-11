/**
 * @file multiplayer_protocol.c
 * @brief Serial frame parser and multiplayer message encoding.
 */

#include "multiplayer_protocol.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum {
  MP_PARSER_WAIT_START,
  MP_PARSER_READ_TYPE,
  MP_PARSER_READ_LEN,
  MP_PARSER_READ_PAYLOAD,
  MP_PARSER_READ_CHECKSUM,
  MP_PARSER_READ_END
} mp_parser_state_t;

static mp_parser_state_t parser_state = MP_PARSER_WAIT_START;
static mp_message_t parser_message;
static uint8_t parser_payload_index = 0;
static uint8_t parser_checksum = 0;
static uint8_t parser_received_checksum = 0;

#define MP_SNAPSHOT_FLAG_ALIVE   0x01
#define MP_SNAPSHOT_FLAG_FOOD    0x02
#define MP_SNAPSHOT_FLAG_POWERUP 0x04
#define MP_SNAPSHOT_FLAG_HELD    0x08
#define MP_SNAPSHOT_FLAG_SHIELD  0x10
#define MP_SNAPSHOT_FLAG_QUEUED  0x20
#define MP_SNAPSHOT_MAX_SNAKE_SEGMENTS 48

static bool mp_protocol_direction_is_valid(direction_t direction) {
  return direction == DIRECTION_UP || direction == DIRECTION_RIGHT || direction == DIRECTION_DOWN || direction == DIRECTION_LEFT;
}

static uint8_t mp_protocol_checksum(const mp_message_t *message) {
  uint8_t checksum = (uint8_t) message->type ^ message->length;

  for (uint8_t i = 0; i < message->length; i++)
    checksum ^= message->payload[i];

  return checksum;
}

static direction_t mp_protocol_infer_direction(const position_t *segments, uint16_t length) {
  if (segments == NULL || length < 2)
    return DIRECTION_RIGHT;

  position_t head = segments[0];
  position_t neck = segments[1];

  if (head.x > neck.x)
    return DIRECTION_RIGHT;

  if (head.x < neck.x)
    return DIRECTION_LEFT;

  if (head.y > neck.y)
    return DIRECTION_DOWN;

  if (head.y < neck.y)
    return DIRECTION_UP;

  return DIRECTION_RIGHT;
}

static uint8_t mp_protocol_clamp_powerup_type(powerup_type_t type) {
  return type < POWERUP_TYPE_COUNT ? (uint8_t) type : 0;
}

static void mp_protocol_write_u16(uint8_t *payload, uint16_t *offset, uint16_t value) {
  payload[(*offset)++] = (uint8_t) (value & 0xFF);
  payload[(*offset)++] = (uint8_t) ((value >> 8) & 0xFF);
}

static void mp_protocol_write_u32(uint8_t *payload, uint16_t *offset, uint32_t value) {
  payload[(*offset)++] = (uint8_t) (value & 0xFF);
  payload[(*offset)++] = (uint8_t) ((value >> 8) & 0xFF);
  payload[(*offset)++] = (uint8_t) ((value >> 16) & 0xFF);
  payload[(*offset)++] = (uint8_t) ((value >> 24) & 0xFF);
}

static int mp_protocol_read_u16(const mp_message_t *message, uint16_t *offset, uint16_t *value) {
  if (message == NULL || offset == NULL || value == NULL)
    return 1;

  if (*offset + 2 > message->length)
    return 1;

  *value = (uint16_t) message->payload[*offset] | ((uint16_t) message->payload[*offset + 1] << 8);
  *offset += 2;

  return 0;
}

static int mp_protocol_read_u32(const mp_message_t *message, uint16_t *offset, uint32_t *value) {
  if (message == NULL || offset == NULL || value == NULL)
    return 1;

  if (*offset + 4 > message->length)
    return 1;

  *value = (uint32_t) message->payload[*offset] | ((uint32_t) message->payload[*offset + 1] << 8) | ((uint32_t) message->payload[*offset + 2] << 16) | ((uint32_t) message->payload[*offset + 3] << 24);
  *offset += 4;

  return 0;
}

static int mp_protocol_snapshot_payload_size(const mp_snapshot_t *snapshot, uint8_t *payload_size) {
  if (snapshot == NULL || payload_size == NULL)
    return 1;

  uint16_t size = 11;

  for (uint8_t i = 0; i < MP_SNAPSHOT_PLAYER_COUNT; i++) {
    uint16_t length = snake_get_length(&snapshot->players[i].snake);

    if (length == 0 || length > UINT8_MAX)
      return 1;

    if (length > MP_SNAPSHOT_MAX_SNAKE_SEGMENTS)
      length = MP_SNAPSHOT_MAX_SNAKE_SEGMENTS;

    size += (uint16_t) (20 + 2 * length);
  }

  if (size > MP_MAX_PAYLOAD_SIZE)
    return 1;

  *payload_size = (uint8_t) size;
  return 0;
}

static int mp_protocol_write_position(uint8_t *payload, uint16_t *offset, position_t position) {
  if (payload == NULL || offset == NULL)
    return 1;

  if (position.x < 0 || position.x > UINT8_MAX || position.y < 0 || position.y > UINT8_MAX)
    return 1;

  payload[(*offset)++] = (uint8_t) position.x;
  payload[(*offset)++] = (uint8_t) position.y;

  return 0;
}

static int mp_protocol_write_player_snapshot(const mp_player_snapshot_t *player, uint8_t *payload, uint16_t *offset) {
  if (player == NULL || payload == NULL || offset == NULL)
    return 1;

  uint8_t flags = 0;

  if (player->alive)
    flags |= MP_SNAPSHOT_FLAG_ALIVE;
  if (player->food_active)
    flags |= MP_SNAPSHOT_FLAG_FOOD;
  if (player->powerup_active)
    flags |= MP_SNAPSHOT_FLAG_POWERUP;
  if (player->held_powerup_active)
    flags |= MP_SNAPSHOT_FLAG_HELD;
  if (player->shield_active)
    flags |= MP_SNAPSHOT_FLAG_SHIELD;
  if (player->queued_powerup_active)
    flags |= MP_SNAPSHOT_FLAG_QUEUED;

  mp_protocol_write_u32(payload, offset, player->score);
  payload[(*offset)++] = flags;

  if (mp_protocol_write_position(payload, offset, player->food_position) != 0)
    return 1;

  if (mp_protocol_write_position(payload, offset, player->powerup_position) != 0)
    return 1;

  payload[(*offset)++] = mp_protocol_clamp_powerup_type(player->powerup_type);
  payload[(*offset)++] = mp_protocol_clamp_powerup_type(player->held_powerup);
  payload[(*offset)++] = mp_protocol_clamp_powerup_type(player->queued_powerup);
  payload[(*offset)++] = player->effect_count > MP_SNAPSHOT_EFFECT_SLOTS ? MP_SNAPSHOT_EFFECT_SLOTS : player->effect_count;

  for (uint8_t i = 0; i < MP_SNAPSHOT_EFFECT_SLOTS; i++) {
    payload[(*offset)++] = i < player->effect_count ? mp_protocol_clamp_powerup_type(player->effect_types[i]) : 0;
    mp_protocol_write_u16(payload, offset, i < player->effect_count ? player->effect_ticks[i] : 0);
  }

  uint16_t length = snake_get_length(&player->snake);

  if (length == 0 || length > UINT8_MAX)
    return 1;

  if (length > MP_SNAPSHOT_MAX_SNAKE_SEGMENTS)
    length = MP_SNAPSHOT_MAX_SNAKE_SEGMENTS;

  payload[(*offset)++] = (uint8_t) length;

  for (uint16_t i = 0; i < length; i++) {
    if (mp_protocol_write_position(payload, offset, snake_get_segment(&player->snake, i)) != 0)
      return 1;
  }

  return 0;
}

static powerup_type_t mp_protocol_read_powerup_type(uint8_t value) {
  if (value >= POWERUP_TYPE_COUNT)
    return POWERUP_DOUBLE_POINTS;

  return (powerup_type_t) value;
}

static int mp_protocol_read_position(const mp_message_t *message, uint16_t *offset, position_t *position) {
  if (message == NULL || offset == NULL || position == NULL)
    return 1;

  if (*offset + 2 > message->length)
    return 1;

  position->x = message->payload[(*offset)++];
  position->y = message->payload[(*offset)++];

  return 0;
}

static int mp_protocol_read_player_snapshot(const mp_message_t *message, uint16_t *offset, uint8_t grid_width, uint8_t grid_height, mp_player_snapshot_t *player) {
  if (message == NULL || offset == NULL || player == NULL)
    return 1;

  uint8_t flags = 0;

  memset(player, 0, sizeof(*player));

  if (mp_protocol_read_u32(message, offset, &player->score) != 0)
    return 1;

  if (*offset >= message->length)
    return 1;

  flags = message->payload[(*offset)++];
  player->alive = (flags & MP_SNAPSHOT_FLAG_ALIVE) != 0;
  player->food_active = (flags & MP_SNAPSHOT_FLAG_FOOD) != 0;
  player->powerup_active = (flags & MP_SNAPSHOT_FLAG_POWERUP) != 0;
  player->held_powerup_active = (flags & MP_SNAPSHOT_FLAG_HELD) != 0;
  player->shield_active = (flags & MP_SNAPSHOT_FLAG_SHIELD) != 0;
  player->queued_powerup_active = (flags & MP_SNAPSHOT_FLAG_QUEUED) != 0;

  if (mp_protocol_read_position(message, offset, &player->food_position) != 0)
    return 1;

  if (mp_protocol_read_position(message, offset, &player->powerup_position) != 0)
    return 1;

  if (*offset + 4 > message->length)
    return 1;

  player->powerup_type = mp_protocol_read_powerup_type(message->payload[(*offset)++]);
  player->held_powerup = mp_protocol_read_powerup_type(message->payload[(*offset)++]);
  player->queued_powerup = mp_protocol_read_powerup_type(message->payload[(*offset)++]);
  player->effect_count = message->payload[(*offset)++];

  if (player->effect_count > MP_SNAPSHOT_EFFECT_SLOTS)
    player->effect_count = MP_SNAPSHOT_EFFECT_SLOTS;

  for (uint8_t i = 0; i < MP_SNAPSHOT_EFFECT_SLOTS; i++) {
    uint16_t ticks = 0;

    if (*offset >= message->length)
      return 1;

    player->effect_types[i] = mp_protocol_read_powerup_type(message->payload[(*offset)++]);

    if (mp_protocol_read_u16(message, offset, &ticks) != 0)
      return 1;

    player->effect_ticks[i] = ticks;
  }

  if (*offset >= message->length)
    return 1;

  uint8_t length = message->payload[(*offset)++];

  if (length == 0)
    return 1;

  if ((uint16_t) *offset + (uint16_t) length * 2 > message->length)
    return 1;

  position_t segments[SNAKE_MAX_SEGMENTS];

  for (uint8_t i = 0; i < length; i++) {
    segments[i].x = message->payload[(*offset)++];
    segments[i].y = message->payload[(*offset)++];
  }

  snake_init(&player->snake, grid_width, grid_height);

  if (snake_set_body(&player->snake, segments, length, mp_protocol_infer_direction(segments, length)) != 0)
    return 1;

  snake_set_alive(&player->snake, player->alive);

  return 0;
}

static void mp_protocol_begin_frame(uint8_t type) {
  memset(&parser_message, 0, sizeof(parser_message));
  parser_message.type = (mp_message_type_t) type;
  parser_payload_index = 0;
  parser_checksum = type;
  parser_received_checksum = 0;
  parser_state = MP_PARSER_READ_LEN;
}

static mp_parse_result_t mp_protocol_parser_error(uint8_t byte) {
  mp_protocol_parser_reset();

  if (byte == MP_START_BYTE)
    parser_state = MP_PARSER_READ_TYPE;

  return MP_PARSE_ERROR;
}

void mp_protocol_parser_reset(void) {
  memset(&parser_message, 0, sizeof(parser_message));
  parser_payload_index = 0;
  parser_checksum = 0;
  parser_received_checksum = 0;
  parser_state = MP_PARSER_WAIT_START;
}

mp_parse_result_t mp_protocol_parse_byte(uint8_t byte, mp_message_t *message) {
  switch (parser_state) {
    case MP_PARSER_WAIT_START:
      if (byte == MP_START_BYTE)
        parser_state = MP_PARSER_READ_TYPE;
      return MP_PARSE_IN_PROGRESS;

    case MP_PARSER_READ_TYPE:
      mp_protocol_begin_frame(byte);
      return MP_PARSE_IN_PROGRESS;

    case MP_PARSER_READ_LEN:
      if (byte > MP_MAX_PAYLOAD_SIZE)
        return mp_protocol_parser_error(byte);

      parser_message.length = byte;
      parser_checksum ^= byte;
      parser_state = byte == 0 ? MP_PARSER_READ_CHECKSUM : MP_PARSER_READ_PAYLOAD;
      return MP_PARSE_IN_PROGRESS;

    case MP_PARSER_READ_PAYLOAD:
      if (parser_payload_index >= MP_MAX_PAYLOAD_SIZE)
        return mp_protocol_parser_error(byte);

      parser_message.payload[parser_payload_index++] = byte;
      parser_checksum ^= byte;

      if (parser_payload_index >= parser_message.length)
        parser_state = MP_PARSER_READ_CHECKSUM;

      return MP_PARSE_IN_PROGRESS;

    case MP_PARSER_READ_CHECKSUM:
      parser_received_checksum = byte;
      parser_state = MP_PARSER_READ_END;
      return MP_PARSE_IN_PROGRESS;

    case MP_PARSER_READ_END:
      if (byte != MP_END_BYTE)
        return mp_protocol_parser_error(byte);

      if (parser_received_checksum != parser_checksum)
        return mp_protocol_parser_error(byte);

      if (message != NULL)
        *message = parser_message;

      mp_protocol_parser_reset();
      return MP_PARSE_COMPLETE;

    default:
      mp_protocol_parser_reset();
      return MP_PARSE_ERROR;
  }
}

int mp_protocol_encode_message(const mp_message_t *message, uint8_t *out, uint16_t out_capacity, uint16_t *out_size) {
  if (message == NULL || out == NULL || out_size == NULL)
    return 1;

  if (message->length > MP_MAX_PAYLOAD_SIZE)
    return 1;

  uint16_t frame_size = (uint16_t) message->length + 5;

  if (out_capacity < frame_size)
    return 1;

  out[0] = MP_START_BYTE;
  out[1] = (uint8_t) message->type;
  out[2] = message->length;

  for (uint8_t i = 0; i < message->length; i++)
    out[3 + i] = message->payload[i];

  out[3 + message->length] = mp_protocol_checksum(message);
  out[4 + message->length] = MP_END_BYTE;
  *out_size = frame_size;

  return 0;
}

int mp_protocol_make_hello(mp_message_t *message) {
  if (message == NULL)
    return 1;

  memset(message, 0, sizeof(*message));
  message->type = MP_MSG_HELLO;
  message->length = 0;

  return 0;
}

int mp_protocol_make_ack(mp_message_t *message) {
  if (message == NULL)
    return 1;

  memset(message, 0, sizeof(*message));
  message->type = MP_MSG_ACK;
  message->length = 0;

  return 0;
}

int mp_protocol_make_input(uint8_t player_id, direction_t direction, bool activate_powerup, mp_message_t *message) {
  if (message == NULL)
    return 1;

  if (player_id == 0)
    return 1;

  if (!mp_protocol_direction_is_valid(direction))
    return 1;

  memset(message, 0, sizeof(*message));
  message->type = MP_MSG_INPUT;
  message->length = 3;
  message->payload[0] = player_id;
  message->payload[1] = (uint8_t) direction;
  message->payload[2] = activate_powerup ? MP_INPUT_FLAG_ACTIVATE_POWERUP : 0;

  return 0;
}

int mp_protocol_read_input(const mp_message_t *message, uint8_t *player_id, direction_t *direction, bool *activate_powerup) {
  if (message == NULL || player_id == NULL || direction == NULL || activate_powerup == NULL)
    return 1;

  if (message->type != MP_MSG_INPUT || message->length < 2)
    return 1;

  if (message->payload[1] > DIRECTION_LEFT)
    return 1;

  *player_id = message->payload[0];
  *direction = (direction_t) message->payload[1];
  *activate_powerup = message->length >= 3 && (message->payload[2] & MP_INPUT_FLAG_ACTIVATE_POWERUP) != 0;

  return 0;
}

int mp_protocol_make_snapshot(const mp_snapshot_t *snapshot, mp_message_t *message) {
  if (snapshot == NULL || message == NULL)
    return 1;

  uint8_t payload_size = 0;

  if (mp_protocol_snapshot_payload_size(snapshot, &payload_size) != 0)
    return 1;

  memset(message, 0, sizeof(*message));
  message->type = MP_MSG_SNAPSHOT;
  message->length = payload_size;

  uint16_t offset = 0;

  mp_protocol_write_u16(message->payload, &offset, snapshot->tick);
  message->payload[offset++] = snapshot->mode;
  message->payload[offset++] = snapshot->phase;
  message->payload[offset++] = snapshot->winner;
  mp_protocol_write_u16(message->payload, &offset, snapshot->countdown_ticks);
  message->payload[offset++] = snapshot->grid_width;
  message->payload[offset++] = snapshot->grid_height;

  for (uint8_t i = 0; i < MP_SNAPSHOT_PLAYER_COUNT; i++)
    message->payload[offset++] = snapshot->player_skins[i];

  for (uint8_t i = 0; i < MP_SNAPSHOT_PLAYER_COUNT; i++) {
    if (mp_protocol_write_player_snapshot(&snapshot->players[i], message->payload, &offset) != 0)
      return 1;
  }

  return offset == message->length ? 0 : 1;
}

int mp_protocol_read_snapshot(const mp_message_t *message, mp_snapshot_t *snapshot) {
  if (message == NULL || snapshot == NULL)
    return 1;

  if (message->type != MP_MSG_SNAPSHOT)
    return 1;

  if (message->length < 11)
    return 1;

  memset(snapshot, 0, sizeof(*snapshot));

  uint16_t offset = 0;

  if (mp_protocol_read_u16(message, &offset, &snapshot->tick) != 0)
    return 1;

  snapshot->mode = message->payload[offset++];
  snapshot->phase = message->payload[offset++];
  snapshot->winner = message->payload[offset++];

  if (mp_protocol_read_u16(message, &offset, &snapshot->countdown_ticks) != 0)
    return 1;

  if (offset + 2 > message->length)
    return 1;

  snapshot->grid_width = message->payload[offset++];
  snapshot->grid_height = message->payload[offset++];

  if (offset + MP_SNAPSHOT_PLAYER_COUNT > message->length)
    return 1;

  for (uint8_t i = 0; i < MP_SNAPSHOT_PLAYER_COUNT; i++)
    snapshot->player_skins[i] = message->payload[offset++];

  if (snapshot->grid_width == 0 || snapshot->grid_height == 0)
    return 1;

  for (uint8_t i = 0; i < MP_SNAPSHOT_PLAYER_COUNT; i++) {
    if (mp_protocol_read_player_snapshot(message, &offset, snapshot->grid_width, snapshot->grid_height, &snapshot->players[i]) != 0)
      return 1;
  }

  return offset == message->length ? 0 : 1;
}

int mp_protocol_make_ping(mp_message_t *message) {
  if (message == NULL)
    return 1;

  memset(message, 0, sizeof(*message));
  message->type = MP_MSG_PING;
  message->length = 0;

  return 0;
}

int mp_protocol_make_join_request(uint16_t code, mp_message_t *message) {
  if (message == NULL)
    return 1;

  if (code < 1000 || code > 9999)
    return 1;

  memset(message, 0, sizeof(*message));
  message->type = MP_MSG_JOIN_REQUEST;
  message->length = 2;
  message->payload[0] = (uint8_t) (code & 0xFF);
  message->payload[1] = (uint8_t) ((code >> 8) & 0xFF);

  return 0;
}

int mp_protocol_read_join_code(const mp_message_t *message, uint16_t *code) {
  if (message == NULL || code == NULL)
    return 1;

  if (message->type != MP_MSG_JOIN_REQUEST || message->length != 2)
    return 1;

  *code = (uint16_t) message->payload[0] | ((uint16_t) message->payload[1] << 8);

  return 0;
}

int mp_protocol_make_join_accept(mp_message_t *message) {
  if (message == NULL)
    return 1;

  memset(message, 0, sizeof(*message));
  message->type = MP_MSG_JOIN_ACCEPT;
  message->length = 0;

  return 0;
}

int mp_protocol_make_join_reject(mp_message_t *message) {
  if (message == NULL)
    return 1;

  memset(message, 0, sizeof(*message));
  message->type = MP_MSG_JOIN_REJECT;
  message->length = 0;

  return 0;
}

int mp_protocol_make_start(uint8_t mode, mp_message_t *message) {
  if (message == NULL)
    return 1;

  memset(message, 0, sizeof(*message));
  message->type = MP_MSG_START;
  message->length = 1;
  message->payload[0] = mode;

  return 0;
}

int mp_protocol_read_start_mode(const mp_message_t *message, uint8_t *mode) {
  if (message == NULL || mode == NULL)
    return 1;

  if (message->type != MP_MSG_START)
    return 1;

  *mode = message->length > 0 ? message->payload[0] : 0;

  return 0;
}

int mp_protocol_make_skin_select(uint8_t player_id, uint8_t skin, mp_message_t *message) {
  if (message == NULL)
    return 1;

  if (player_id == 0)
    return 1;

  memset(message, 0, sizeof(*message));
  message->type = MP_MSG_SKIN_SELECT;
  message->length = 2;
  message->payload[0] = player_id;
  message->payload[1] = skin;

  return 0;
}

int mp_protocol_read_skin_select(const mp_message_t *message, uint8_t *player_id, uint8_t *skin) {
  if (message == NULL || player_id == NULL || skin == NULL)
    return 1;

  if (message->type != MP_MSG_SKIN_SELECT || message->length < 2)
    return 1;

  *player_id = message->payload[0];
  *skin = message->payload[1];

  return 0;
}
