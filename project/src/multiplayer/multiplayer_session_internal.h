#ifndef MULTIPLAYER_SESSION_INTERNAL_H
#define MULTIPLAYER_SESSION_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include "multiplayer_protocol.h"
#include "multiplayer_session.h"
#include "powerup.h"

/**
 * @file multiplayer_session_internal.h
 * @brief Shared internal state for the multiplayer session implementation.
 *
 * This header is intentionally private to the multiplayer subsystem. It exposes
 * host/snapshot storage and helper hooks split across session, rules and
 * accessor modules.
 */

#define MP_PLAYER_1 1
#define MP_PLAYER_2 2
#define MP_FRAME_MAX_SIZE (MP_MAX_PAYLOAD_SIZE + 5)
#define MP_MIN_ROOM_CODE 1000
#define MP_MAX_ROOM_CODE 9999
#define MP_COUNTDOWN_TICKS_PER_SECOND 60
#define MP_SNAPSHOT_PERIOD_TICKS 8
#define MP_ARENA_VISIBILITY_DISTANCE 12

#define MULTIPLAYER_COUNTDOWN_SECONDS (MULTIPLAYER_COUNTDOWN_TICKS / MP_COUNTDOWN_TICKS_PER_SECOND)
#define MULTIPLAYER_MAX_SYNCED_SNAKE_SEGMENTS 48
#define MULTIPLAYER_ARENA_POWERUP_SCORE 5


extern multiplayer_role_t current_role;
extern multiplayer_connection_state_t connection_state;
extern multiplayer_game_mode_t current_game_mode;
extern multiplayer_map_t current_map;
extern multiplayer_match_phase_t match_phase;

extern snake_t host_player1;
extern snake_t host_player2;
extern snake_t snapshot_player1;
extern snake_t snapshot_player2;
extern position_t previous_player1_body[SNAKE_MAX_SEGMENTS];
extern position_t previous_player2_body[SNAKE_MAX_SEGMENTS];
extern uint16_t previous_player1_length;
extern uint16_t previous_player2_length;
extern food_t host_food;
extern food_t snapshot_food;
extern food_t host_powerup;
extern food_t snapshot_powerup;
extern food_t host_score_food[2];
extern food_t snapshot_score_food[2];
extern position_t host_food_animation_positions[2][MULTIPLAYER_FOOD_ANIMATION_MAX_POSITIONS];
extern uint8_t host_food_animation_position_counts[2];
extern position_t snapshot_food_animation_positions[2][MULTIPLAYER_FOOD_ANIMATION_MAX_POSITIONS];
extern uint8_t snapshot_food_animation_position_counts[2];
extern powerup_t host_player_powerups[2];
extern powerup_t snapshot_player_powerups[2];
extern uint8_t host_player_skins[2];
extern uint8_t snapshot_player_skins[2];

/**
 * @brief Timed powerup effect slot stored by the host and snapshots.
 */
typedef struct {
  powerup_type_t type; /**< Effect type. */
  uint16_t ticks;      /**< Remaining duration in timer ticks. */
  bool active;         /**< Whether the slot is currently used. */
} multiplayer_effect_t;

extern bool host_held_powerup_active[2];
extern powerup_type_t host_held_powerup[2];
extern bool snapshot_held_powerup_active[2];
extern powerup_type_t snapshot_held_powerup[2];
extern bool host_queued_powerup_active[2];
extern powerup_type_t host_queued_powerup[2];
extern bool snapshot_queued_powerup_active[2];
extern powerup_type_t snapshot_queued_powerup[2];
extern bool host_shield_active[2];
extern bool snapshot_shield_active[2];
extern multiplayer_effect_t host_effects[2][MULTIPLAYER_EFFECT_SLOTS];
extern multiplayer_effect_t snapshot_effects[2][MULTIPLAYER_EFFECT_SLOTS];
extern uint16_t player_death_animation_tick[2];
extern uint8_t movement_tick_counter;
extern uint8_t countdown_tick_counter;
extern uint8_t countdown_seconds;
extern uint8_t snapshot_tick_counter;
extern uint16_t host_tick;
extern uint16_t last_snapshot_tick;
extern bool snapshot_available;
extern uint16_t player1_score;
extern uint16_t player2_score;
extern uint8_t winner;
extern uint16_t death_animation_tick;

extern bool remote_input_seen;
extern direction_t last_remote_direction;
extern direction_t last_local_direction;
extern bool last_sent_direction_valid;
extern direction_t last_sent_direction;

extern uint16_t host_room_code;
extern uint16_t last_generated_room_code;
extern uint16_t room_code_sequence;
extern uint16_t pending_join_code;
extern bool join_rejected;

/**
 * @brief Get active multiplayer grid width for the current mode.
 *
 * @return Width in cells.
 */
uint16_t multiplayer_session_active_grid_columns(void);

/**
 * @brief Get active multiplayer grid height for the current mode.
 *
 * @return Height in cells.
 */
uint16_t multiplayer_session_active_grid_rows(void);

/**
 * @brief Initialize all host entities for a new match.
 *
 * @note Mutates host snakes, food, powerups, scores, timers and snapshots.
 */
void multiplayer_session_prepare_match_entities(void);

/**
 * @brief Send an authoritative host snapshot to the peer.
 *
 * @note Also updates local snapshot copies from host state.
 */
void multiplayer_session_send_snapshot(void);

/**
 * @brief Store host snake bodies before the next movement step.
 */
void multiplayer_session_store_previous_snakes(void);

/**
 * @brief Activate the currently held powerup for a player.
 *
 * @param player_id Public player identifier (1 or 2).
 */
void multiplayer_session_activate_player_powerup(uint8_t player_id);

/**
 * @brief Store initial food positions for the current movement animation.
 */
void multiplayer_session_store_host_food_animation_starts(void);

/**
 * @brief Append a food animation position in host state.
 *
 * @param index Food slot index.
 * @param position Position to append.
 */
void multiplayer_session_append_host_food_animation_position(uint8_t index, position_t position);

/**
 * @brief Reset a host food animation slot to the current food position.
 *
 * @param index Food slot index.
 */
void multiplayer_session_reset_host_food_animation_slot(uint8_t index);

#endif
