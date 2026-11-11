/**
 * @file multiplayer_session_accessors.c
 * @brief Renderer-facing getters for multiplayer host and snapshot state.
 */

#include "multiplayer_session_internal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static uint8_t multiplayer_session_index_from_player(uint8_t player_id) {
  return player_id == MP_PLAYER_2 ? 1 : 0;
}

static const snake_t *multiplayer_session_select_player_snake(uint8_t player_id) {
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

static const food_t *multiplayer_session_select_player_food(uint8_t player_id) {
  uint8_t index = multiplayer_session_index_from_player(player_id);

  if (current_role == MP_ROLE_HOST)
    return &host_score_food[index];

  if (current_role == MP_ROLE_CLIENT && snapshot_available)
    return &snapshot_score_food[index];

  return NULL;
}

static const powerup_t *multiplayer_session_select_player_powerup(uint8_t player_id) {
  uint8_t index = multiplayer_session_index_from_player(player_id);

  if (current_role == MP_ROLE_HOST)
    return &host_player_powerups[index];

  if (current_role == MP_ROLE_CLIENT && snapshot_available)
    return &snapshot_player_powerups[index];

  return NULL;
}

multiplayer_role_t multiplayer_session_get_role(void) {
  return current_role;
}

multiplayer_connection_state_t multiplayer_session_get_connection_state(void) {
  return connection_state;
}

uint16_t multiplayer_session_get_room_code(void) {
  if (current_role == MP_ROLE_HOST && (host_room_code < MP_MIN_ROOM_CODE || host_room_code > MP_MAX_ROOM_CODE)) {
    uint16_t range = (uint16_t) (MP_MAX_ROOM_CODE - MP_MIN_ROOM_CODE + 1);

    room_code_sequence++;
    host_room_code = (uint16_t) (MP_MIN_ROOM_CODE + ((uint32_t) room_code_sequence * 7919u + (uint32_t) host_tick * 37u) % range);
  }

  return host_room_code;
}

bool multiplayer_session_join_was_rejected(void) {
  return join_rejected;
}

multiplayer_game_mode_t multiplayer_session_get_mode(void) {
  return current_game_mode;
}

multiplayer_game_mode_t multiplayer_session_get_game_mode(void) {
  return current_game_mode;
}

multiplayer_map_t multiplayer_session_get_map(void) {
  return current_map;
}

multiplayer_game_phase_t multiplayer_session_get_phase(void) {
  return match_phase;
}

multiplayer_match_phase_t multiplayer_session_get_match_phase(void) {
  return match_phase;
}

uint16_t multiplayer_session_get_countdown_ticks(void) {
  return (uint16_t) countdown_seconds * MP_COUNTDOWN_TICKS_PER_SECOND;
}

uint8_t multiplayer_session_get_countdown_seconds(void) {
  return countdown_seconds;
}

uint32_t multiplayer_session_get_player_score(uint8_t player_id) {
  if (player_id == MP_PLAYER_1)
    return player1_score;

  if (player_id == MP_PLAYER_2)
    return player2_score;

  return 0;
}

multiplayer_winner_t multiplayer_session_get_winner(void) {
  if (winner == MP_PLAYER_1)
    return MP_WINNER_PLAYER_1;

  if (winner == MP_PLAYER_2)
    return MP_WINNER_PLAYER_2;

  if (winner == MP_WINNER_TIE)
    return MP_WINNER_TIE;

  return MP_WINNER_NONE;
}

uint16_t multiplayer_session_get_grid_columns(void) {
  return multiplayer_session_active_grid_columns();
}

uint16_t multiplayer_session_get_grid_rows(void) {
  return multiplayer_session_active_grid_rows();
}

bool multiplayer_session_can_start(void) {
  return current_role == MP_ROLE_HOST && match_phase == MP_MATCH_LOBBY && connection_state == MP_CONNECTION_CONNECTED;
}

bool multiplayer_session_has_snapshot(void) {
  return snapshot_available;
}

uint16_t multiplayer_session_get_last_snapshot_tick(void) {
  return last_snapshot_tick;
}

bool multiplayer_session_remote_input_seen(void) {
  return remote_input_seen;
}

direction_t multiplayer_session_get_last_remote_direction(void) {
  return last_remote_direction;
}

direction_t multiplayer_session_get_last_local_direction(void) {
  return last_local_direction;
}

uint16_t multiplayer_session_get_previous_snake_length(uint8_t player_id) {
  if (player_id == MP_PLAYER_1)
    return previous_player1_length;

  if (player_id == MP_PLAYER_2)
    return previous_player2_length;

  return 0;
}

position_t multiplayer_session_get_previous_snake_segment(uint8_t player_id, uint16_t index) {
  position_t empty = {0, 0};

  if (player_id == MP_PLAYER_1 && index < previous_player1_length)
    return previous_player1_body[index];

  if (player_id == MP_PLAYER_2 && index < previous_player2_length)
    return previous_player2_body[index];

  return empty;
}

uint8_t multiplayer_session_get_animation_step(void) {
  return movement_tick_counter;
}

uint8_t multiplayer_session_get_animation_steps(void) {
  return MULTIPLAYER_MOVE_TICKS > 1 ? MULTIPLAYER_MOVE_TICKS - 1 : 1;
}

uint16_t multiplayer_session_get_death_animation_tick(void) {
  return death_animation_tick;
}

uint16_t multiplayer_session_get_player_death_animation_tick(uint8_t player_id) {
  uint8_t index = multiplayer_session_index_from_player(player_id);

  if (current_role == MP_ROLE_HOST || current_role == MP_ROLE_CLIENT)
    return player_death_animation_tick[index];

  return death_animation_tick;
}

uint16_t multiplayer_session_get_death_animation_ticks(void) {
  return MULTIPLAYER_DEATH_ANIMATION_TICKS;
}

bool multiplayer_session_player_is_alive(uint8_t player_id) {
  const snake_t *snake = multiplayer_session_select_player_snake(player_id);
  return snake != NULL && snake_is_alive(snake);
}

bool multiplayer_session_player_food_active(uint8_t player_id) {
  const food_t *food = multiplayer_session_select_player_food(player_id);
  return food != NULL && food_is_active(food);
}

position_t multiplayer_session_get_player_food_position(uint8_t player_id) {
  const food_t *food = multiplayer_session_select_player_food(player_id);

  if (food == NULL)
    return (position_t) {0, 0};

  return food_get_position(food);
}


uint8_t multiplayer_session_get_food_animation_position_count(uint8_t player_id) {
  uint8_t index = multiplayer_session_index_from_player(player_id);

  if (current_role == MP_ROLE_CLIENT && !snapshot_available)
    return 0;

  if (current_role == MP_ROLE_HOST)
    return host_food_animation_position_counts[index];

  if (current_role == MP_ROLE_CLIENT)
    return snapshot_food_animation_position_counts[index];

  return 0;
}

position_t multiplayer_session_get_food_animation_position(uint8_t player_id, uint8_t index) {
  position_t empty = {0, 0};
  uint8_t player_index = multiplayer_session_index_from_player(player_id);
  const food_t *food = multiplayer_session_select_player_food(player_id);

  if (current_role == MP_ROLE_HOST) {
    if (host_food_animation_position_counts[player_index] == 0)
      return food != NULL ? food_get_position(food) : empty;

    if (index >= host_food_animation_position_counts[player_index])
      index = host_food_animation_position_counts[player_index] - 1;

    return host_food_animation_positions[player_index][index];
  }

  if (current_role == MP_ROLE_CLIENT && snapshot_available) {
    if (snapshot_food_animation_position_counts[player_index] == 0)
      return food != NULL ? food_get_position(food) : empty;

    if (index >= snapshot_food_animation_position_counts[player_index])
      index = snapshot_food_animation_position_counts[player_index] - 1;

    return snapshot_food_animation_positions[player_index][index];
  }

  return empty;
}

bool multiplayer_session_player_powerup_active(uint8_t player_id) {
  const powerup_t *powerup = multiplayer_session_select_player_powerup(player_id);
  return powerup != NULL && powerup_is_active(powerup);
}

position_t multiplayer_session_get_player_powerup_position(uint8_t player_id) {
  const powerup_t *powerup = multiplayer_session_select_player_powerup(player_id);

  if (powerup == NULL)
    return (position_t) {0, 0};

  return powerup_get_position(powerup);
}

powerup_type_t multiplayer_session_get_player_powerup_type(uint8_t player_id) {
  const powerup_t *powerup = multiplayer_session_select_player_powerup(player_id);

  if (powerup == NULL)
    return POWERUP_DOUBLE_POINTS;

  return powerup_get_type(powerup);
}

bool multiplayer_session_player_has_held_powerup(uint8_t player_id) {
  uint8_t index = multiplayer_session_index_from_player(player_id);

  if (current_role == MP_ROLE_HOST)
    return host_held_powerup_active[index];

  if (current_role == MP_ROLE_CLIENT && snapshot_available)
    return snapshot_held_powerup_active[index];

  return false;
}

powerup_type_t multiplayer_session_get_player_held_powerup(uint8_t player_id) {
  uint8_t index = multiplayer_session_index_from_player(player_id);

  if (current_role == MP_ROLE_HOST)
    return host_held_powerup[index];

  if (current_role == MP_ROLE_CLIENT && snapshot_available)
    return snapshot_held_powerup[index];

  return POWERUP_DOUBLE_POINTS;
}


bool multiplayer_session_player_has_queued_powerup(uint8_t player_id) {
  uint8_t index = multiplayer_session_index_from_player(player_id);

  if (current_role == MP_ROLE_HOST)
    return host_queued_powerup_active[index];

  if (current_role == MP_ROLE_CLIENT && snapshot_available)
    return snapshot_queued_powerup_active[index];

  return false;
}

powerup_type_t multiplayer_session_get_player_queued_powerup(uint8_t player_id) {
  uint8_t index = multiplayer_session_index_from_player(player_id);

  if (current_role == MP_ROLE_HOST)
    return host_queued_powerup[index];

  if (current_role == MP_ROLE_CLIENT && snapshot_available)
    return snapshot_queued_powerup[index];

  return POWERUP_DOUBLE_POINTS;
}

bool multiplayer_session_player_has_shield(uint8_t player_id) {
  uint8_t index = multiplayer_session_index_from_player(player_id);

  if (current_role == MP_ROLE_HOST)
    return host_shield_active[index];

  if (current_role == MP_ROLE_CLIENT && snapshot_available)
    return snapshot_shield_active[index];

  return false;
}

uint8_t multiplayer_session_get_player_effect_count(uint8_t player_id) {
  uint8_t index = multiplayer_session_index_from_player(player_id);
  const multiplayer_effect_t (*effects)[MULTIPLAYER_EFFECT_SLOTS] = current_role == MP_ROLE_HOST ? host_effects : snapshot_effects;
  uint8_t count = 0;

  if (current_role == MP_ROLE_CLIENT && !snapshot_available)
    return 0;

  for (uint8_t i = 0; i < MULTIPLAYER_EFFECT_SLOTS; i++) {
    if (effects[index][i].active)
      count++;
  }

  return count;
}

powerup_type_t multiplayer_session_get_player_effect_type(uint8_t player_id, uint8_t index) {
  uint8_t player_index = multiplayer_session_index_from_player(player_id);
  const multiplayer_effect_t (*effects)[MULTIPLAYER_EFFECT_SLOTS] = current_role == MP_ROLE_HOST ? host_effects : snapshot_effects;

  if (index >= MULTIPLAYER_EFFECT_SLOTS)
    return POWERUP_DOUBLE_POINTS;

  if (current_role == MP_ROLE_CLIENT && !snapshot_available)
    return POWERUP_DOUBLE_POINTS;

  if (!effects[player_index][index].active)
    return POWERUP_DOUBLE_POINTS;

  return effects[player_index][index].type;
}

uint16_t multiplayer_session_get_player_effect_ticks(uint8_t player_id, uint8_t index) {
  uint8_t player_index = multiplayer_session_index_from_player(player_id);
  const multiplayer_effect_t (*effects)[MULTIPLAYER_EFFECT_SLOTS] = current_role == MP_ROLE_HOST ? host_effects : snapshot_effects;

  if (index >= MULTIPLAYER_EFFECT_SLOTS)
    return 0;

  if (current_role == MP_ROLE_CLIENT && !snapshot_available)
    return 0;

  if (!effects[player_index][index].active)
    return 0;

  return effects[player_index][index].ticks;
}
