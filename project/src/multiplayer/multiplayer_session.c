/**
 * @file multiplayer_session.c
 * @brief Multiplayer connection, lobby, snapshots and message handling.
 */

#include "multiplayer_session.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "multiplayer_protocol.h"
#include "serial.h"
#include "timer.h"
#include "assets.h"
#include "multiplayer_session_internal.h"
#include "magnet.h"

multiplayer_role_t current_role = MP_ROLE_NONE;
multiplayer_connection_state_t connection_state = MP_CONNECTION_DISCONNECTED;
multiplayer_game_mode_t current_game_mode = MP_GAME_MODE_SCORE;
multiplayer_map_t current_map = MP_MAP_ARENA;
multiplayer_match_phase_t match_phase = MP_MATCH_LOBBY;

snake_t host_player1;
snake_t host_player2;
snake_t snapshot_player1;
snake_t snapshot_player2;
position_t previous_player1_body[SNAKE_MAX_SEGMENTS];
position_t previous_player2_body[SNAKE_MAX_SEGMENTS];
uint16_t previous_player1_length = 0;
uint16_t previous_player2_length = 0;
food_t host_food;
food_t snapshot_food;
food_t host_powerup;
food_t snapshot_powerup;
food_t host_score_food[2];
food_t snapshot_score_food[2];
position_t host_food_animation_positions[2][MULTIPLAYER_FOOD_ANIMATION_MAX_POSITIONS];
uint8_t host_food_animation_position_counts[2] = {0, 0};
position_t snapshot_food_animation_positions[2][MULTIPLAYER_FOOD_ANIMATION_MAX_POSITIONS];
uint8_t snapshot_food_animation_position_counts[2] = {0, 0};
powerup_t host_player_powerups[2];
powerup_t snapshot_player_powerups[2];
uint8_t host_player_skins[2] = {0, 1};
uint8_t snapshot_player_skins[2] = {0, 1};
bool host_held_powerup_active[2] = {false, false};
powerup_type_t host_held_powerup[2] = {POWERUP_DOUBLE_POINTS, POWERUP_DOUBLE_POINTS};
bool snapshot_held_powerup_active[2] = {false, false};
powerup_type_t snapshot_held_powerup[2] = {POWERUP_DOUBLE_POINTS, POWERUP_DOUBLE_POINTS};
bool host_queued_powerup_active[2] = {false, false};
powerup_type_t host_queued_powerup[2] = {POWERUP_DOUBLE_POINTS, POWERUP_DOUBLE_POINTS};
bool snapshot_queued_powerup_active[2] = {false, false};
powerup_type_t snapshot_queued_powerup[2] = {POWERUP_DOUBLE_POINTS, POWERUP_DOUBLE_POINTS};
bool host_shield_active[2] = {false, false};
bool snapshot_shield_active[2] = {false, false};
multiplayer_effect_t host_effects[2][MULTIPLAYER_EFFECT_SLOTS];
multiplayer_effect_t snapshot_effects[2][MULTIPLAYER_EFFECT_SLOTS];
uint16_t player_death_animation_tick[2] = {0, 0};
uint8_t movement_tick_counter = 0;
uint8_t countdown_tick_counter = 0;
uint8_t countdown_seconds = MULTIPLAYER_COUNTDOWN_SECONDS;
uint8_t snapshot_tick_counter = 0;
uint16_t host_tick = 0;
uint16_t last_snapshot_tick = 0;
bool snapshot_available = false;
uint16_t player1_score = 0;
uint16_t player2_score = 0;
uint8_t winner = 0;
uint16_t death_animation_tick = 0;

bool remote_input_seen = false;
direction_t last_remote_direction = DIRECTION_RIGHT;
direction_t last_local_direction = DIRECTION_RIGHT;
bool last_sent_direction_valid = false;
direction_t last_sent_direction = DIRECTION_RIGHT;

uint16_t host_room_code = 0;
uint16_t last_generated_room_code = 0;
uint16_t room_code_sequence = 0;
uint16_t pending_join_code = 0;
bool join_rejected = false;


static bool multiplayer_session_positions_equal(position_t first, position_t second) {
  return first.x == second.x && first.y == second.y;
}

static void multiplayer_session_set_animation_single(position_t positions[2][MULTIPLAYER_FOOD_ANIMATION_MAX_POSITIONS], uint8_t counts[2], uint8_t index, position_t position) {
  if (index >= 2)
    return;

  positions[index][0] = position;
  counts[index] = 1;
}

static void multiplayer_session_append_animation_position(position_t positions[2][MULTIPLAYER_FOOD_ANIMATION_MAX_POSITIONS], uint8_t counts[2], uint8_t index, position_t position) {
  if (index >= 2)
    return;

  if (counts[index] == 0) {
    multiplayer_session_set_animation_single(positions, counts, index, position);
    return;
  }

  if (multiplayer_session_positions_equal(positions[index][counts[index] - 1], position))
    return;

  if (counts[index] < MULTIPLAYER_FOOD_ANIMATION_MAX_POSITIONS) {
    positions[index][counts[index]] = position;
    counts[index]++;
    return;
  }

  positions[index][MULTIPLAYER_FOOD_ANIMATION_MAX_POSITIONS - 1] = position;
}

void multiplayer_session_reset_host_food_animation_slot(uint8_t index) {
  if (index >= 2)
    return;

  multiplayer_session_set_animation_single(host_food_animation_positions,
                                           host_food_animation_position_counts,
                                           index,
                                           food_get_position(&host_score_food[index]));
}

void multiplayer_session_store_host_food_animation_starts(void) {
  for (uint8_t i = 0; i < 2; i++)
    multiplayer_session_reset_host_food_animation_slot(i);
}

void multiplayer_session_append_host_food_animation_position(uint8_t index, position_t position) {
  multiplayer_session_append_animation_position(host_food_animation_positions,
                                                host_food_animation_position_counts,
                                                index,
                                                position);
}

static void multiplayer_session_copy_host_food_animation_to_snapshot(void) {
  for (uint8_t i = 0; i < 2; i++) {
    snapshot_food_animation_position_counts[i] = host_food_animation_position_counts[i];

    if (snapshot_food_animation_position_counts[i] == 0) {
      multiplayer_session_set_animation_single(snapshot_food_animation_positions,
                                               snapshot_food_animation_position_counts,
                                               i,
                                               food_get_position(&snapshot_score_food[i]));
      continue;
    }

    for (uint8_t j = 0; j < snapshot_food_animation_position_counts[i] && j < MULTIPLAYER_FOOD_ANIMATION_MAX_POSITIONS; j++)
      snapshot_food_animation_positions[i][j] = host_food_animation_positions[i][j];
  }
}

static void multiplayer_session_store_snapshot_food_transition(uint8_t index, position_t previous_position, position_t current_position) {
  if (index >= 2)
    return;

  snapshot_food_animation_positions[index][0] = previous_position;
  snapshot_food_animation_position_counts[index] = 1;

  if (!multiplayer_session_positions_equal(previous_position, current_position))
    multiplayer_session_append_animation_position(snapshot_food_animation_positions,
                                                  snapshot_food_animation_position_counts,
                                                  index,
                                                  current_position);
}

static uint16_t multiplayer_session_generate_room_code(void) {
  room_code_sequence++;

  uint32_t seed = timer_get_counter();
  seed ^= ((uint32_t) room_code_sequence * 97u);
  seed ^= ((uint32_t) host_tick << 5);
  seed = seed * 1103515245u + 12345u;

  uint16_t code = (uint16_t) (MP_MIN_ROOM_CODE + (seed % (MP_MAX_ROOM_CODE - MP_MIN_ROOM_CODE + 1)));

  if (code == last_generated_room_code) {
    code++;

    if (code > MP_MAX_ROOM_CODE)
      code = MP_MIN_ROOM_CODE;
  }

  last_generated_room_code = code;
  return code;
}

void multiplayer_session_store_previous_snakes(void) {
  previous_player1_length = snake_get_length(&host_player1);
  previous_player2_length = snake_get_length(&host_player2);

  for (uint16_t i = 0; i < previous_player1_length && i < SNAKE_MAX_SEGMENTS; i++)
    previous_player1_body[i] = snake_get_segment(&host_player1, i);

  for (uint16_t i = 0; i < previous_player2_length && i < SNAKE_MAX_SEGMENTS; i++)
    previous_player2_body[i] = snake_get_segment(&host_player2, i);
}

static void multiplayer_session_update_snapshot_from_host(void) {
  snapshot_player1 = host_player1;
  snapshot_player2 = host_player2;
  snapshot_food = host_score_food[0];

  if (powerup_is_active(&host_player_powerups[0])) {
    snapshot_powerup.position = powerup_get_position(&host_player_powerups[0]);
    snapshot_powerup.active = true;
  }
  else
    snapshot_powerup.active = false;

  for (uint8_t i = 0; i < 2; i++) {
    snapshot_score_food[i] = host_score_food[i];
    snapshot_player_powerups[i] = host_player_powerups[i];
    snapshot_player_skins[i] = host_player_skins[i];
    snapshot_held_powerup_active[i] = host_held_powerup_active[i];
    snapshot_held_powerup[i] = host_held_powerup[i];
    snapshot_queued_powerup_active[i] = host_queued_powerup_active[i];
    snapshot_queued_powerup[i] = host_queued_powerup[i];
    snapshot_shield_active[i] = host_shield_active[i];

    for (uint8_t j = 0; j < MULTIPLAYER_EFFECT_SLOTS; j++)
      snapshot_effects[i][j] = host_effects[i][j];
  }

  multiplayer_session_copy_host_food_animation_to_snapshot();

  last_snapshot_tick = host_tick;
  snapshot_available = true;
}

static void multiplayer_session_store_previous_from_snapshot(void) {
  previous_player1_length = snake_get_length(&snapshot_player1);
  previous_player2_length = snake_get_length(&snapshot_player2);

  for (uint16_t i = 0; i < previous_player1_length && i < SNAKE_MAX_SEGMENTS; i++)
    previous_player1_body[i] = snake_get_segment(&snapshot_player1, i);

  for (uint16_t i = 0; i < previous_player2_length && i < SNAKE_MAX_SEGMENTS; i++)
    previous_player2_body[i] = snake_get_segment(&snapshot_player2, i);
}

static int multiplayer_session_send_message(const mp_message_t *message) {
  uint8_t frame[MP_FRAME_MAX_SIZE];
  uint16_t frame_size = 0;

  if (mp_protocol_encode_message(message, frame, sizeof(frame), &frame_size) != 0)
    return 1;

  return serial_send_bytes(frame, frame_size);
}

static void multiplayer_session_send_ack(void) {
  mp_message_t message;

  if (mp_protocol_make_ack(&message) != 0)
    return;

  (void) multiplayer_session_send_message(&message);
}

static void multiplayer_session_send_input(direction_t direction, bool activate_powerup) {
  mp_message_t message;

  if (connection_state != MP_CONNECTION_CONNECTED || match_phase != MP_MATCH_PLAYING)
    return;

  if (mp_protocol_make_input(MP_PLAYER_2, direction, activate_powerup, &message) != 0)
    return;

  if (multiplayer_session_send_message(&message) == 0) {
    last_sent_direction = direction;
    last_sent_direction_valid = true;
  }
}

void multiplayer_session_send_snapshot(void) {
  mp_message_t message;
  mp_snapshot_t snapshot;

  multiplayer_session_update_snapshot_from_host();

  memset(&snapshot, 0, sizeof(snapshot));
  snapshot.tick = host_tick;
  snapshot.mode = (uint8_t) current_game_mode;
  snapshot.phase = (uint8_t) match_phase;
  snapshot.winner = winner;
  snapshot.countdown_ticks = (uint16_t) countdown_seconds * MP_COUNTDOWN_TICKS_PER_SECOND;
  snapshot.grid_width = (uint8_t) multiplayer_session_active_grid_columns();
  snapshot.grid_height = (uint8_t) multiplayer_session_active_grid_rows();

  snapshot.player_skins[0] = host_player_skins[0];
  snapshot.player_skins[1] = host_player_skins[1];

  snapshot.players[0].snake = host_player1;
  snapshot.players[0].score = player1_score;
  snapshot.players[0].alive = snake_is_alive(&host_player1);
  snapshot.players[0].food_active = food_is_active(&host_score_food[0]);
  snapshot.players[0].food_position = food_get_position(&host_score_food[0]);
  snapshot.players[0].powerup_active = powerup_is_active(&host_player_powerups[0]);
  snapshot.players[0].powerup_position = powerup_get_position(&host_player_powerups[0]);
  snapshot.players[0].powerup_type = powerup_get_type(&host_player_powerups[0]);
  snapshot.players[0].held_powerup_active = host_held_powerup_active[0];
  snapshot.players[0].held_powerup = host_held_powerup[0];
  snapshot.players[0].queued_powerup_active = host_queued_powerup_active[0];
  snapshot.players[0].queued_powerup = host_queued_powerup[0];
  snapshot.players[0].shield_active = host_shield_active[0];

  snapshot.players[1].snake = host_player2;
  snapshot.players[1].score = player2_score;
  snapshot.players[1].alive = snake_is_alive(&host_player2);
  snapshot.players[1].food_active = food_is_active(&host_score_food[1]);
  snapshot.players[1].food_position = food_get_position(&host_score_food[1]);
  snapshot.players[1].powerup_active = powerup_is_active(&host_player_powerups[1]);
  snapshot.players[1].powerup_position = powerup_get_position(&host_player_powerups[1]);
  snapshot.players[1].powerup_type = powerup_get_type(&host_player_powerups[1]);
  snapshot.players[1].held_powerup_active = host_held_powerup_active[1];
  snapshot.players[1].held_powerup = host_held_powerup[1];
  snapshot.players[1].queued_powerup_active = host_queued_powerup_active[1];
  snapshot.players[1].queued_powerup = host_queued_powerup[1];
  snapshot.players[1].shield_active = host_shield_active[1];

  for (uint8_t player = 0; player < 2; player++) {
    uint8_t count = 0;

    for (uint8_t slot = 0; slot < MULTIPLAYER_EFFECT_SLOTS; slot++) {
      if (!host_effects[player][slot].active)
        continue;

      snapshot.players[player].effect_types[count] = host_effects[player][slot].type;
      snapshot.players[player].effect_ticks[count] = host_effects[player][slot].ticks;
      count++;

      if (count >= MP_SNAPSHOT_EFFECT_SLOTS)
        break;
    }

    snapshot.players[player].effect_count = count;
  }

  if (connection_state != MP_CONNECTION_CONNECTED)
    return;

  if (mp_protocol_make_snapshot(&snapshot, &message) != 0)
    return;

  (void) multiplayer_session_send_message(&message);
}

static void multiplayer_session_send_join_request(void) {
  mp_message_t message;

  if (pending_join_code < MP_MIN_ROOM_CODE || pending_join_code > MP_MAX_ROOM_CODE)
    return;

  if (mp_protocol_make_join_request(pending_join_code, &message) != 0)
    return;

  (void) multiplayer_session_send_message(&message);
}

static void multiplayer_session_send_join_accept(void) {
  mp_message_t message;

  if (mp_protocol_make_join_accept(&message) != 0)
    return;

  (void) multiplayer_session_send_message(&message);
}

static void multiplayer_session_send_join_reject(void) {
  mp_message_t message;

  if (mp_protocol_make_join_reject(&message) != 0)
    return;

  (void) multiplayer_session_send_message(&message);
}

static void multiplayer_session_send_start(void) {
  mp_message_t message;

  if (mp_protocol_make_start((uint8_t) current_game_mode, &message) != 0)
    return;

  (void) multiplayer_session_send_message(&message);
}


static bool multiplayer_session_skin_is_valid(uint8_t skin) {
  return skin < MULTIPLAYER_SNAKE_SKIN_COUNT;
}

static uint8_t multiplayer_session_unique_skin(uint8_t requested, uint8_t other_skin, int8_t step) {
  uint8_t skin = multiplayer_session_skin_is_valid(requested) ? requested : 0;

  if (step == 0)
    step = 1;

  for (uint8_t tries = 0; tries < MULTIPLAYER_SNAKE_SKIN_COUNT; tries++) {
    if (skin != other_skin)
      return skin;

    if (step > 0)
      skin = (uint8_t) ((skin + 1) % MULTIPLAYER_SNAKE_SKIN_COUNT);
    else
      skin = skin == 0 ? (uint8_t) (MULTIPLAYER_SNAKE_SKIN_COUNT - 1) : (uint8_t) (skin - 1);
  }

  return other_skin == 0 ? 1 : 0;
}

static void multiplayer_session_publish_skin_snapshot(void) {
  snapshot_player_skins[0] = host_player_skins[0];
  snapshot_player_skins[1] = host_player_skins[1];
}

static void multiplayer_session_host_set_player_skin(uint8_t player_id, uint8_t requested_skin, int8_t step) {
  if (player_id != MP_PLAYER_1 && player_id != MP_PLAYER_2)
    return;

  uint8_t index = player_id == MP_PLAYER_2 ? 1 : 0;
  uint8_t other = index == 0 ? 1 : 0;
  host_player_skins[index] = multiplayer_session_unique_skin(requested_skin, host_player_skins[other], step);
  multiplayer_session_publish_skin_snapshot();
}

static void multiplayer_session_send_skin_select(uint8_t player_id, uint8_t skin) {
  mp_message_t message;

  if (connection_state != MP_CONNECTION_CONNECTED || match_phase != MP_MATCH_LOBBY)
    return;

  if (mp_protocol_make_skin_select(player_id, skin, &message) != 0)
    return;

  (void) multiplayer_session_send_message(&message);
}

static void multiplayer_session_handle_join_request(const mp_message_t *message) {
  uint16_t received_code = 0;

  if (current_role != MP_ROLE_HOST)
    return;

  if (mp_protocol_read_join_code(message, &received_code) != 0)
    return;

  if (received_code == host_room_code) {
    connection_state = MP_CONNECTION_CONNECTED;
    match_phase = MP_MATCH_LOBBY;
    multiplayer_session_send_join_accept();
    multiplayer_session_send_snapshot();
    return;
  }

  multiplayer_session_send_join_reject();
}

static void multiplayer_session_handle_input_message(const mp_message_t *message) {
  uint8_t player_id = 0;
  direction_t direction = DIRECTION_RIGHT;
  bool activate_powerup = false;

  if (mp_protocol_read_input(message, &player_id, &direction, &activate_powerup) != 0)
    return;

  if (current_role == MP_ROLE_HOST && player_id == MP_PLAYER_2 && connection_state == MP_CONNECTION_CONNECTED && match_phase == MP_MATCH_PLAYING) {
    snake_set_next_direction(&host_player2, direction);

    if (activate_powerup)
      multiplayer_session_activate_player_powerup(MP_PLAYER_2);

    last_remote_direction = direction;
    remote_input_seen = true;
  }
}

static void multiplayer_session_handle_snapshot_message(const mp_message_t *message) {
  mp_snapshot_t snapshot;

  if (current_role != MP_ROLE_CLIENT)
    return;

  if (mp_protocol_read_snapshot(message, &snapshot) != 0)
    return;

  bool was_alive[2] = {
    snapshot_available && snake_is_alive(&snapshot_player1),
    snapshot_available && snake_is_alive(&snapshot_player2)
  };

  if (snapshot_available)
    multiplayer_session_store_previous_from_snapshot();
  else {
    previous_player1_length = snake_get_length(&snapshot.players[0].snake);
    previous_player2_length = snake_get_length(&snapshot.players[1].snake);

    for (uint16_t i = 0; i < previous_player1_length && i < SNAKE_MAX_SEGMENTS; i++)
      previous_player1_body[i] = snake_get_segment(&snapshot.players[0].snake, i);

    for (uint16_t i = 0; i < previous_player2_length && i < SNAKE_MAX_SEGMENTS; i++)
      previous_player2_body[i] = snake_get_segment(&snapshot.players[1].snake, i);
  }

  movement_tick_counter = 0;
  multiplayer_match_phase_t previous_phase = match_phase;
  snapshot_player1 = snapshot.players[0].snake;
  snapshot_player2 = snapshot.players[1].snake;

  bool now_alive[2] = {
    snake_is_alive(&snapshot_player1),
    snake_is_alive(&snapshot_player2)
  };

  for (uint8_t i = 0; i < 2; i++) {
    if (now_alive[i])
      player_death_animation_tick[i] = 0;
    else if (!snapshot_available || was_alive[i])
      player_death_animation_tick[i] = 0;
  }

  last_snapshot_tick = snapshot.tick;
  snapshot_player_skins[0] = snapshot.player_skins[0] < MULTIPLAYER_SNAKE_SKIN_COUNT ? snapshot.player_skins[0] : 0;
  snapshot_player_skins[1] = snapshot.player_skins[1] < MULTIPLAYER_SNAKE_SKIN_COUNT ? snapshot.player_skins[1] : 1;
  if (snapshot_player_skins[0] == snapshot_player_skins[1])
    snapshot_player_skins[1] = (uint8_t) ((snapshot_player_skins[0] + 1) % MULTIPLAYER_SNAKE_SKIN_COUNT);

  player1_score = (uint16_t) snapshot.players[0].score;
  player2_score = (uint16_t) snapshot.players[1].score;
  winner = snapshot.winner;
  countdown_seconds = (uint8_t) ((snapshot.countdown_ticks + MP_COUNTDOWN_TICKS_PER_SECOND - 1) / MP_COUNTDOWN_TICKS_PER_SECOND);

  if (snapshot.mode <= MP_GAME_MODE_ARENA)
    current_game_mode = (multiplayer_game_mode_t) snapshot.mode;

  if (snapshot.phase <= MP_MATCH_FINISHED) {
    match_phase = (multiplayer_match_phase_t) snapshot.phase;
    if (match_phase == MP_MATCH_FINISHED && previous_phase != MP_MATCH_FINISHED)
      death_animation_tick = 0;
  }

  food_init(&snapshot_food, multiplayer_session_active_grid_columns(), multiplayer_session_active_grid_rows());
  snapshot_food.position = snapshot.players[0].food_position;
  snapshot_food.active = snapshot.players[0].food_active;

  position_t previous_food_positions[2];
  bool previous_food_active[2];

  for (uint8_t i = 0; i < 2; i++) {
    previous_food_positions[i] = food_get_position(&snapshot_score_food[i]);
    previous_food_active[i] = snapshot_available && food_is_active(&snapshot_score_food[i]);
  }

  for (uint8_t i = 0; i < 2; i++) {
    food_init(&snapshot_score_food[i], multiplayer_session_active_grid_columns(), multiplayer_session_active_grid_rows());
    snapshot_score_food[i].position = snapshot.players[i].food_position;
    snapshot_score_food[i].active = snapshot.players[i].food_active;

    if (previous_food_active[i] && snapshot.players[i].food_active)
      multiplayer_session_store_snapshot_food_transition(i, previous_food_positions[i], snapshot.players[i].food_position);
    else
      multiplayer_session_set_animation_single(snapshot_food_animation_positions,
                                               snapshot_food_animation_position_counts,
                                               i,
                                               snapshot.players[i].food_position);

    powerup_init(&snapshot_player_powerups[i], multiplayer_session_active_grid_columns(), multiplayer_session_active_grid_rows());
    snapshot_player_powerups[i].position = snapshot.players[i].powerup_position;
    snapshot_player_powerups[i].type = snapshot.players[i].powerup_type;
    snapshot_player_powerups[i].active = snapshot.players[i].powerup_active;

    snapshot_held_powerup_active[i] = snapshot.players[i].held_powerup_active;
    snapshot_held_powerup[i] = snapshot.players[i].held_powerup;
    snapshot_queued_powerup_active[i] = snapshot.players[i].queued_powerup_active;
    snapshot_queued_powerup[i] = snapshot.players[i].queued_powerup;
    snapshot_shield_active[i] = snapshot.players[i].shield_active;

    for (uint8_t j = 0; j < MULTIPLAYER_EFFECT_SLOTS; j++) {
      snapshot_effects[i][j].active = j < snapshot.players[i].effect_count;
      snapshot_effects[i][j].type = snapshot.players[i].effect_types[j];
      snapshot_effects[i][j].ticks = snapshot.players[i].effect_ticks[j];
    }
  }

  food_init(&snapshot_powerup, multiplayer_session_active_grid_columns(), multiplayer_session_active_grid_rows());
  snapshot_powerup.position = snapshot.players[0].powerup_position;
  snapshot_powerup.active = snapshot.players[0].powerup_active;

  snapshot_available = true;

  if (!join_rejected)
    connection_state = MP_CONNECTION_CONNECTED;
}

static void multiplayer_session_handle_skin_select_message(const mp_message_t *message) {
  uint8_t player_id = 0;
  uint8_t skin = 0;

  if (current_role != MP_ROLE_HOST || connection_state != MP_CONNECTION_CONNECTED || match_phase != MP_MATCH_LOBBY)
    return;

  if (mp_protocol_read_skin_select(message, &player_id, &skin) != 0)
    return;

  if (player_id != MP_PLAYER_2)
    return;

  multiplayer_session_host_set_player_skin(MP_PLAYER_2, skin, 1);
  multiplayer_session_send_snapshot();
}

static void multiplayer_session_handle_start_message(const mp_message_t *message) {
  uint8_t game_mode = 0;

  if (current_role != MP_ROLE_CLIENT)
    return;

  if (mp_protocol_read_start_mode(message, &game_mode) != 0)
    return;

  if (game_mode <= MP_GAME_MODE_ARENA)
    current_game_mode = (multiplayer_game_mode_t) game_mode;

  current_map = MP_MAP_ARENA;
  match_phase = MP_MATCH_COUNTDOWN;
  countdown_seconds = MULTIPLAYER_COUNTDOWN_SECONDS;
  connection_state = MP_CONNECTION_CONNECTED;
  join_rejected = false;
}

static void multiplayer_session_handle_message(const mp_message_t *message) {
  if (message == NULL)
    return;

  switch (message->type) {
    case MP_MSG_HELLO:
      if (current_role == MP_ROLE_HOST) {
        connection_state = MP_CONNECTION_CONNECTED;
        match_phase = MP_MATCH_LOBBY;
        multiplayer_session_send_ack();
        multiplayer_session_send_snapshot();
      }
      break;

    case MP_MSG_ACK:
      if (!join_rejected)
        connection_state = MP_CONNECTION_CONNECTED;
      break;

    case MP_MSG_JOIN_REQUEST:
      multiplayer_session_handle_join_request(message);
      break;

    case MP_MSG_JOIN_ACCEPT:
      if (current_role == MP_ROLE_CLIENT) {
        join_rejected = false;
        connection_state = MP_CONNECTION_CONNECTED;
        match_phase = MP_MATCH_LOBBY;
      }
      break;

    case MP_MSG_JOIN_REJECT:
      if (current_role == MP_ROLE_CLIENT) {
        join_rejected = true;
        connection_state = MP_CONNECTION_DISCONNECTED;
        snapshot_available = false;
      }
      break;

    case MP_MSG_START:
      multiplayer_session_handle_start_message(message);
      break;

    case MP_MSG_SKIN_SELECT:
      multiplayer_session_handle_skin_select_message(message);
      break;

    case MP_MSG_INPUT:
      multiplayer_session_handle_input_message(message);
      break;

    case MP_MSG_SNAPSHOT:
      multiplayer_session_handle_snapshot_message(message);
      break;

    case MP_MSG_PING:
      multiplayer_session_send_ack();
      break;

    default:
      break;
  }
}

void multiplayer_session_reset(multiplayer_role_t role) {
  current_role = role;
  connection_state = role == MP_ROLE_NONE ? MP_CONNECTION_DISCONNECTED : MP_CONNECTION_WAITING;
  match_phase = MP_MATCH_LOBBY;
  movement_tick_counter = 0;
  countdown_tick_counter = 0;
  countdown_seconds = MULTIPLAYER_COUNTDOWN_SECONDS;
  snapshot_tick_counter = 0;
  host_tick = 0;
  last_snapshot_tick = 0;
  snapshot_available = false;
  remote_input_seen = false;
  last_remote_direction = DIRECTION_LEFT;
  last_local_direction = DIRECTION_RIGHT;
  last_sent_direction_valid = false;
  last_sent_direction = DIRECTION_RIGHT;
  join_rejected = false;
  player1_score = 0;
  player2_score = 0;
  winner = 0;
  death_animation_tick = 0;
  host_player_skins[0] = 0;
  host_player_skins[1] = 1;
  snapshot_player_skins[0] = host_player_skins[0];
  snapshot_player_skins[1] = host_player_skins[1];

  mp_protocol_parser_reset();
  snake_init(&snapshot_player1, MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);
  snake_init(&snapshot_player2, MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);
  food_init(&snapshot_food, MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);
  food_init(&snapshot_powerup, MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);
  food_init(&host_food, MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);
  food_init(&host_powerup, MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);
  for (uint8_t i = 0; i < 2; i++) {
    food_init(&host_score_food[i], MULTIPLAYER_SCORE_GRID_COLUMNS, MULTIPLAYER_SCORE_GRID_ROWS);
    food_init(&snapshot_score_food[i], MULTIPLAYER_SCORE_GRID_COLUMNS, MULTIPLAYER_SCORE_GRID_ROWS);
    powerup_init(&host_player_powerups[i], MULTIPLAYER_SCORE_GRID_COLUMNS, MULTIPLAYER_SCORE_GRID_ROWS);
    powerup_init(&snapshot_player_powerups[i], MULTIPLAYER_SCORE_GRID_COLUMNS, MULTIPLAYER_SCORE_GRID_ROWS);
    host_held_powerup_active[i] = false;
    snapshot_held_powerup_active[i] = false;
    host_held_powerup[i] = POWERUP_DOUBLE_POINTS;
    snapshot_held_powerup[i] = POWERUP_DOUBLE_POINTS;
    host_queued_powerup_active[i] = false;
    snapshot_queued_powerup_active[i] = false;
    host_queued_powerup[i] = POWERUP_DOUBLE_POINTS;
    snapshot_queued_powerup[i] = POWERUP_DOUBLE_POINTS;
    host_shield_active[i] = false;
    snapshot_shield_active[i] = false;
    player_death_animation_tick[i] = 0;
    multiplayer_session_set_animation_single(host_food_animation_positions,
                                             host_food_animation_position_counts,
                                             i,
                                             food_get_position(&host_score_food[i]));
    multiplayer_session_set_animation_single(snapshot_food_animation_positions,
                                             snapshot_food_animation_position_counts,
                                             i,
                                             food_get_position(&snapshot_score_food[i]));

    for (uint8_t j = 0; j < MULTIPLAYER_EFFECT_SLOTS; j++) {
      host_effects[i][j].active = false;
      host_effects[i][j].ticks = 0;
      host_effects[i][j].type = POWERUP_DOUBLE_POINTS;
      snapshot_effects[i][j] = host_effects[i][j];
    }
  }

  if (role == MP_ROLE_HOST) {
    host_room_code = multiplayer_session_generate_room_code();
    multiplayer_session_prepare_match_entities();
  }
  else if (role == MP_ROLE_CLIENT) {
    multiplayer_session_send_join_request();
  }
  else {
    host_room_code = 0;
  }
}

void multiplayer_session_shutdown(void) {
  multiplayer_session_reset(MP_ROLE_NONE);
}

void multiplayer_session_set_pending_join_code(uint16_t code) {
  pending_join_code = code;
  join_rejected = false;
}




const food_t *multiplayer_session_get_food(void) {
  if (current_role == MP_ROLE_HOST)
    return &host_score_food[0];

  if (current_role == MP_ROLE_CLIENT && snapshot_available)
    return &snapshot_score_food[0];

  return NULL;
}

const food_t *multiplayer_session_get_powerup(void) {
  if (current_role == MP_ROLE_HOST)
    return &host_powerup;

  if (current_role == MP_ROLE_CLIENT && snapshot_available)
    return &snapshot_powerup;

  return NULL;
}




void multiplayer_session_select_next_game_mode(void) {
  if (current_role != MP_ROLE_HOST || match_phase != MP_MATCH_LOBBY)
    return;

  current_game_mode = current_game_mode == MP_GAME_MODE_SCORE ? MP_GAME_MODE_ARENA : MP_GAME_MODE_SCORE;
  multiplayer_session_prepare_match_entities();

  if (connection_state == MP_CONNECTION_CONNECTED)
    multiplayer_session_send_snapshot();
}

void multiplayer_session_select_next_map(void) {
  if (current_role != MP_ROLE_HOST || match_phase != MP_MATCH_LOBBY)
    return;

  current_map = MP_MAP_ARENA;

  if (connection_state == MP_CONNECTION_CONNECTED)
    multiplayer_session_send_snapshot();
}

void multiplayer_session_request_start(void) {
  if (!multiplayer_session_can_start())
    return;

  multiplayer_session_prepare_match_entities();
  match_phase = MP_MATCH_COUNTDOWN;
  countdown_seconds = MULTIPLAYER_COUNTDOWN_SECONDS;
  countdown_tick_counter = 0;
  snapshot_tick_counter = 0;
  death_animation_tick = 0;

  if (connection_state == MP_CONNECTION_CONNECTED) {
    multiplayer_session_send_start();
    multiplayer_session_send_snapshot();
  }
  else
    multiplayer_session_send_snapshot();
}

const char *multiplayer_session_game_mode_to_string(multiplayer_game_mode_t mode) {
  switch (mode) {
    case MP_GAME_MODE_SCORE:
      return "SCORE RACE";

    case MP_GAME_MODE_ARENA:
      return "ARENA BATTLE";

    default:
      return "UNKNOWN";
  }
}

const char *multiplayer_session_map_to_string(multiplayer_map_t map) {
  switch (map) {
    case MP_MAP_ARENA:
      return "ARENA";

    default:
      return "UNKNOWN";
  }
}

const char *multiplayer_session_match_phase_to_string(multiplayer_match_phase_t phase) {
  switch (phase) {
    case MP_MATCH_LOBBY:
      return "LOBBY";

    case MP_MATCH_COUNTDOWN:
      return "COUNTDOWN";

    case MP_MATCH_PLAYING:
      return "PLAYING";

    case MP_MATCH_FINISHED:
      return "FINISHED";

    default:
      return "UNKNOWN";
  }
}

void multiplayer_session_select_mode(multiplayer_game_mode_t mode) {
  if (current_role != MP_ROLE_HOST || match_phase != MP_MATCH_LOBBY)
    return;

  if (mode >= MP_MODE_COUNT)
    return;

  current_game_mode = mode;
  multiplayer_session_prepare_match_entities();

  if (connection_state == MP_CONNECTION_CONNECTED)
    multiplayer_session_send_snapshot();
}

void multiplayer_session_toggle_mode(void) {
  multiplayer_session_select_next_game_mode();
}

uint8_t multiplayer_session_get_player_skin(uint8_t player_id) {
  uint8_t index = player_id == MP_PLAYER_2 ? 1 : 0;

  if (current_role == MP_ROLE_CLIENT && snapshot_available)
    return snapshot_player_skins[index];

  return host_player_skins[index];
}

const char *multiplayer_session_get_player_skin_name(uint8_t player_id) {
  return assets_get_snake_skin_name(multiplayer_session_get_player_skin(player_id));
}

static void multiplayer_session_select_player_skin_delta(uint8_t player_id, int8_t step) {
  if (match_phase != MP_MATCH_LOBBY)
    return;

  if (current_role == MP_ROLE_HOST) {
    if (player_id != MP_PLAYER_1)
      return;

    uint8_t index = 0;
    uint8_t current = host_player_skins[index];
    uint8_t requested = step > 0 ? (uint8_t) ((current + 1) % MULTIPLAYER_SNAKE_SKIN_COUNT)
                                : (current == 0 ? (uint8_t) (MULTIPLAYER_SNAKE_SKIN_COUNT - 1) : (uint8_t) (current - 1));

    multiplayer_session_host_set_player_skin(MP_PLAYER_1, requested, step);

    if (connection_state == MP_CONNECTION_CONNECTED)
      multiplayer_session_send_snapshot();

    return;
  }

  if (current_role == MP_ROLE_CLIENT) {
    if (player_id != MP_PLAYER_2)
      return;

    uint8_t current = snapshot_player_skins[1];
    uint8_t other = snapshot_player_skins[0];
    uint8_t requested = step > 0 ? (uint8_t) ((current + 1) % MULTIPLAYER_SNAKE_SKIN_COUNT)
                                : (current == 0 ? (uint8_t) (MULTIPLAYER_SNAKE_SKIN_COUNT - 1) : (uint8_t) (current - 1));

    snapshot_player_skins[1] = multiplayer_session_unique_skin(requested, other, step);
    multiplayer_session_send_skin_select(MP_PLAYER_2, snapshot_player_skins[1]);
  }
}

void multiplayer_session_select_next_player_skin(uint8_t player_id) {
  multiplayer_session_select_player_skin_delta(player_id, 1);
}

void multiplayer_session_select_previous_player_skin(uint8_t player_id) {
  multiplayer_session_select_player_skin_delta(player_id, -1);
}

void multiplayer_session_start_game(void) {
  multiplayer_session_request_start();
}

void multiplayer_session_return_to_lobby(void) {
  if (current_role != MP_ROLE_HOST)
    return;

  match_phase = MP_MATCH_LOBBY;
  winner = MP_WINNER_NONE;
  death_animation_tick = 0;
  countdown_seconds = MULTIPLAYER_COUNTDOWN_SECONDS;
  multiplayer_session_prepare_match_entities();

  if (connection_state == MP_CONNECTION_CONNECTED)
    multiplayer_session_send_snapshot();
}

void multiplayer_session_handle_local_activate_powerup(void) {
  if (match_phase != MP_MATCH_PLAYING)
    return;

  if (current_role == MP_ROLE_HOST) {
    multiplayer_session_activate_player_powerup(MP_PLAYER_1);
    return;
  }

  if (current_role == MP_ROLE_CLIENT) {
    multiplayer_session_send_input(last_local_direction, true);
    return;
  }
}

void multiplayer_session_process_serial_rx(void) {
  uint8_t byte = 0;
  mp_message_t message;

  while (serial_rx_available()) {
    if (serial_receive_byte(&byte) != 0)
      return;

    mp_parse_result_t result = mp_protocol_parse_byte(byte, &message);

    if (result == MP_PARSE_COMPLETE)
      multiplayer_session_handle_message(&message);
  }
}

void multiplayer_session_handle_local_direction(direction_t direction) {
  if (direction > DIRECTION_LEFT)
    return;

  last_local_direction = direction;

  if (match_phase != MP_MATCH_PLAYING)
    return;

  if (current_role == MP_ROLE_HOST) {
    snake_set_next_direction(&host_player1, direction);
    return;
  }

  if (current_role != MP_ROLE_CLIENT)
    return;

  if (last_sent_direction_valid && last_sent_direction == direction)
    return;

  multiplayer_session_send_input(direction, false);
}

const snake_t *multiplayer_session_get_player_snake(uint8_t player_id) {
  switch (player_id) {
    case MP_PLAYER_1:
      if (current_role == MP_ROLE_HOST)
        return &host_player1;

      if (current_role == MP_ROLE_CLIENT && snapshot_available)
        return &snapshot_player1;

      return NULL;

    case MP_PLAYER_2:
      if (current_role == MP_ROLE_HOST)
        return &host_player2;

      if (current_role == MP_ROLE_CLIENT && snapshot_available)
        return &snapshot_player2;

      return NULL;

    default:
      return NULL;
  }
}
