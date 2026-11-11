#ifndef MULTIPLAYER_SESSION_H
#define MULTIPLAYER_SESSION_H

#include <stdbool.h>
#include <stdint.h>

#include "food.h"
#include "powerup.h"
#include "score.h"
#include "snake.h"

/**
 * @file multiplayer_session.h
 * @brief Public multiplayer session model and event API.
 *
 * The session coordinates connection state, lobby choices, match phases,
 * host-authoritative game logic, client snapshots and renderer-facing getters.
 */

#define MULTIPLAYER_SCORE_GRID_COLUMNS 44
#define MULTIPLAYER_SCORE_GRID_ROWS 38
#define MULTIPLAYER_ARENA_GRID_COLUMNS 96
#define MULTIPLAYER_ARENA_GRID_ROWS 72
#define MULTIPLAYER_GRID_COLUMNS MULTIPLAYER_ARENA_GRID_COLUMNS
#define MULTIPLAYER_GRID_ROWS MULTIPLAYER_ARENA_GRID_ROWS
#define MULTIPLAYER_CELL_SIZE 16
#define MULTIPLAYER_MOVE_TICKS 8
#define MULTIPLAYER_SPEED_MOVE_TICKS 5
#define MULTIPLAYER_SLOW_MOVE_TICKS 12
#define MULTIPLAYER_POWERUP_SPAWN_TICKS 360
#define MULTIPLAYER_COUNTDOWN_TICKS 180
#define MULTIPLAYER_DEATH_ANIMATION_TICKS 90
#define MULTIPLAYER_DEATH_IMPACT_TICKS 18
#define MULTIPLAYER_EFFECT_SLOTS 2
#define MULTIPLAYER_SPEED_BOOST_TICKS 360
#define MULTIPLAYER_SLOW_ENEMY_TICKS 300
#define MULTIPLAYER_GHOST_TICKS 240
#define MULTIPLAYER_MAGNET_TICKS 480
#define MULTIPLAYER_FRENZY_TICKS 360
#define MULTIPLAYER_BONUS_FOOD_POINTS 3
#define MULTIPLAYER_ARENA_POWERUP_GROWTH 3
#define MULTIPLAYER_ARENA_FOOD_COUNT 2
#define MULTIPLAYER_SCORE_FOOD_COUNT 1
#define MULTIPLAYER_SNAKE_SKIN_COUNT 5
#define MULTIPLAYER_FOOD_ANIMATION_MAX_POSITIONS 3

/**
 * @brief Local role in a multiplayer session.
 */
typedef enum {
  MP_ROLE_NONE,   /**< Multiplayer inactive. */
  MP_ROLE_HOST,   /**< This machine owns simulation and sends snapshots. */
  MP_ROLE_CLIENT  /**< This machine sends input and receives snapshots. */
} multiplayer_role_t;

/**
 * @brief Serial connection state for multiplayer.
 */
typedef enum {
  MP_CONNECTION_DISCONNECTED, /**< No active accepted connection. */
  MP_CONNECTION_WAITING,      /**< Waiting for handshake or join response. */
  MP_CONNECTION_CONNECTED     /**< Peer has been accepted/acknowledged. */
} multiplayer_connection_state_t;

/**
 * @brief Multiplayer rule set selected in the lobby.
 */
typedef enum {
  MP_MODE_SCORE_RACE,   /**< Separate score race grids with score-based result. */
  MP_MODE_ARENA_BATTLE, /**< Shared arena with player-to-player collisions. */
  MP_MODE_COUNT         /**< Number of game modes. */
} multiplayer_game_mode_t;

/**
 * @brief Runtime phase of a multiplayer match.
 */
typedef enum {
  MP_PHASE_LOBBY,     /**< Lobby and room setup. */
  MP_PHASE_COUNTDOWN, /**< Countdown before movement starts. */
  MP_PHASE_PLAYING,   /**< Active match simulation. */
  MP_PHASE_GAME_OVER  /**< Match finished, result visible. */
} multiplayer_game_phase_t;

#define MP_GAME_MODE_SCORE MP_MODE_SCORE_RACE
#define MP_GAME_MODE_ARENA MP_MODE_ARENA_BATTLE
typedef multiplayer_game_phase_t multiplayer_match_phase_t;
#define MP_MATCH_LOBBY MP_PHASE_LOBBY
#define MP_MATCH_COUNTDOWN MP_PHASE_COUNTDOWN
#define MP_MATCH_PLAYING MP_PHASE_PLAYING
#define MP_MATCH_FINISHED MP_PHASE_GAME_OVER

/**
 * @brief Map identifier used by legacy renderer helpers.
 */
typedef enum {
  MP_MAP_ARENA /**< Static arena layout. */
} multiplayer_map_t;

/**
 * @brief Match result identifier.
 */
typedef enum {
  MP_WINNER_NONE,     /**< No winner yet. */
  MP_WINNER_PLAYER_1, /**< Player 1 won. */
  MP_WINNER_PLAYER_2, /**< Player 2 won. */
  MP_WINNER_TIE       /**< Match ended in a tie. */
} multiplayer_winner_t;

/**
 * @brief Reset the multiplayer session for a role.
 *
 * @param role Role to activate.
 *
 * @note Clears snapshots, scores, effects, pending input and parser state.
 */
void multiplayer_session_reset(multiplayer_role_t role);

/**
 * @brief Shutdown the active multiplayer session.
 *
 * @note Resets the session to MP_ROLE_NONE.
 */
void multiplayer_session_shutdown(void);

/** @brief Get the local role. @return Current role. */
multiplayer_role_t multiplayer_session_get_role(void);

/** @brief Get the current connection state. @return Connection state. */
multiplayer_connection_state_t multiplayer_session_get_connection_state(void);

/** @brief Get the selected game mode. @return Current game mode. */
multiplayer_game_mode_t multiplayer_session_get_mode(void);

/** @brief Get the current match phase. @return Current phase. */
multiplayer_game_phase_t multiplayer_session_get_phase(void);

/** @brief Get the current winner. @return Winner identifier. */
multiplayer_winner_t multiplayer_session_get_winner(void);

/** @brief Get the host room code. @return Four-digit room code, or 0 if absent. */
uint16_t multiplayer_session_get_room_code(void);

/** @brief Check whether the last join request was rejected. @return true if rejected. */
bool multiplayer_session_join_was_rejected(void);

/**
 * @brief Store the room code that a client should request.
 *
 * @param code Room code typed in the join screen.
 */
void multiplayer_session_set_pending_join_code(uint16_t code);

/**
 * @brief Select a multiplayer mode in the lobby.
 *
 * @param mode Requested mode.
 *
 * @note Only the host can change the authoritative mode.
 */
void multiplayer_session_select_mode(multiplayer_game_mode_t mode);

/** @brief Toggle between available lobby modes. */
void multiplayer_session_toggle_mode(void);

/**
 * @brief Get a player's selected skin.
 *
 * @param player_id Player identifier (1 or 2).
 * @return Skin index.
 */
uint8_t multiplayer_session_get_player_skin(uint8_t player_id);

/**
 * @brief Get the display name for a player's skin.
 *
 * @param player_id Player identifier (1 or 2).
 * @return Constant skin name.
 */
const char *multiplayer_session_get_player_skin_name(uint8_t player_id);

/**
 * @brief Select the next available skin for a player.
 *
 * @param player_id Player identifier (1 or 2).
 *
 * @note Host changes are local and authoritative; client changes are sent to host.
 */
void multiplayer_session_select_next_player_skin(uint8_t player_id);

/**
 * @brief Select the previous available skin for a player.
 *
 * @param player_id Player identifier (1 or 2).
 */
void multiplayer_session_select_previous_player_skin(uint8_t player_id);

/**
 * @brief Start the selected multiplayer match from the lobby.
 *
 * @note Host prepares match entities, sends START and begins countdown.
 */
void multiplayer_session_start_game(void);

/**
 * @brief Return from game-over to the lobby.
 *
 * @note Intended for host-controlled post-match flow.
 */
void multiplayer_session_return_to_lobby(void);

/**
 * @brief Consume pending serial bytes and apply complete messages.
 */
void multiplayer_session_process_serial_rx(void);

/**
 * @brief Advance countdown, movement, effects, snapshots and death animation.
 *
 * @note On the host this mutates the authoritative match. On the client it only
 * advances local interpolation/animation timers.
 */
void multiplayer_session_handle_timer(void);

/**
 * @brief Handle a local direction input.
 *
 * @param direction Requested direction.
 *
 * @note Host applies player 1 input directly; client sends player 2 input.
 */
void multiplayer_session_handle_local_direction(direction_t direction);

/**
 * @brief Handle local request to activate the held powerup.
 *
 * @note Host applies immediately for player 1; client sends activation input.
 */
void multiplayer_session_handle_local_activate_powerup(void);

/**
 * @brief Get a player's snake from host state or latest snapshot.
 *
 * @param player_id Player identifier (1 or 2).
 * @return Pointer to snake data.
 */
const snake_t *multiplayer_session_get_player_snake(uint8_t player_id);

/** @brief Get a player's score. @param player_id Player identifier. @return Score. */
uint32_t multiplayer_session_get_player_score(uint8_t player_id);

/** @brief Check whether a player is alive. @param player_id Player identifier. @return true if alive. */
bool multiplayer_session_player_is_alive(uint8_t player_id);

/** @brief Check whether a player's food is active. @param player_id Player identifier. @return true if active. */
bool multiplayer_session_player_food_active(uint8_t player_id);

/** @brief Get a player's food position. @param player_id Player identifier. @return Food position. */
position_t multiplayer_session_get_player_food_position(uint8_t player_id);

/** @brief Get food animation position count. @param player_id Player identifier. @return Position count. */
uint8_t multiplayer_session_get_food_animation_position_count(uint8_t player_id);

/**
 * @brief Get one food animation position.
 *
 * @param player_id Player identifier.
 * @param index Position index.
 * @return Stored animation position.
 */
position_t multiplayer_session_get_food_animation_position(uint8_t player_id, uint8_t index);

/** @brief Check whether a player's powerup pickup is active. @param player_id Player identifier. @return true if active. */
bool multiplayer_session_player_powerup_active(uint8_t player_id);

/** @brief Get a player's powerup pickup position. @param player_id Player identifier. @return Powerup position. */
position_t multiplayer_session_get_player_powerup_position(uint8_t player_id);

/** @brief Get a player's powerup pickup type. @param player_id Player identifier. @return Powerup type. */
powerup_type_t multiplayer_session_get_player_powerup_type(uint8_t player_id);

/** @brief Check whether a player has a held powerup. @param player_id Player identifier. @return true if present. */
bool multiplayer_session_player_has_held_powerup(uint8_t player_id);

/** @brief Get a player's held powerup. @param player_id Player identifier. @return Powerup type. */
powerup_type_t multiplayer_session_get_player_held_powerup(uint8_t player_id);

/** @brief Check whether a player has a queued powerup. @param player_id Player identifier. @return true if present. */
bool multiplayer_session_player_has_queued_powerup(uint8_t player_id);

/** @brief Get a player's queued powerup. @param player_id Player identifier. @return Powerup type. */
powerup_type_t multiplayer_session_get_player_queued_powerup(uint8_t player_id);

/** @brief Check whether a player has an armed shield. @param player_id Player identifier. @return true if shielded. */
bool multiplayer_session_player_has_shield(uint8_t player_id);

/** @brief Get active effect count for a player. @param player_id Player identifier. @return Effect count. */
uint8_t multiplayer_session_get_player_effect_count(uint8_t player_id);

/**
 * @brief Get an active effect type for a player.
 *
 * @param player_id Player identifier.
 * @param index Active effect index.
 * @return Powerup effect type.
 */
powerup_type_t multiplayer_session_get_player_effect_type(uint8_t player_id, uint8_t index);

/**
 * @brief Get remaining ticks for an active player effect.
 *
 * @param player_id Player identifier.
 * @param index Active effect index.
 * @return Remaining ticks.
 */
uint16_t multiplayer_session_get_player_effect_ticks(uint8_t player_id, uint8_t index);

/** @brief Get active grid width. @return Width in cells. */
uint16_t multiplayer_session_get_grid_columns(void);

/** @brief Get active grid height. @return Height in cells. */
uint16_t multiplayer_session_get_grid_rows(void);

/** @brief Get remaining countdown ticks. @return Countdown in ticks. */
uint16_t multiplayer_session_get_countdown_ticks(void);

/** @brief Check whether a client/renderer has snapshot data. @return true if available. */
bool multiplayer_session_has_snapshot(void);

/** @brief Get the tick number of the latest snapshot. @return Snapshot tick. */
uint16_t multiplayer_session_get_last_snapshot_tick(void);

/** @brief Check whether host has received remote input. @return true if seen. */
bool multiplayer_session_remote_input_seen(void);

/** @brief Get the last remote direction received. @return Direction. */
direction_t multiplayer_session_get_last_remote_direction(void);

/** @brief Get the last local direction handled. @return Direction. */
direction_t multiplayer_session_get_last_local_direction(void);

/** @brief Get previous snake length for interpolation. @param player_id Player identifier. @return Previous length. */
uint16_t multiplayer_session_get_previous_snake_length(uint8_t player_id);

/**
 * @brief Get a previous snake segment for interpolation.
 *
 * @param player_id Player identifier.
 * @param index Segment index.
 * @return Previous segment position.
 */
position_t multiplayer_session_get_previous_snake_segment(uint8_t player_id, uint16_t index);

/** @brief Get movement interpolation step. @return Current animation step. */
uint8_t multiplayer_session_get_animation_step(void);

/** @brief Get total movement interpolation steps. @return Animation step count. */
uint8_t multiplayer_session_get_animation_steps(void);

/** @brief Get global death animation tick. @return Tick value. */
uint16_t multiplayer_session_get_death_animation_tick(void);

/** @brief Get death animation tick for a player. @param player_id Player identifier. @return Tick value. */
uint16_t multiplayer_session_get_player_death_animation_tick(uint8_t player_id);

/** @brief Get death animation duration. @return Duration in ticks. */
uint16_t multiplayer_session_get_death_animation_ticks(void);

/* Legacy names / helpers used by the refactored renderer modules. */
/** @brief Legacy alias for multiplayer_session_get_mode(). @return Current game mode. */
multiplayer_game_mode_t multiplayer_session_get_game_mode(void);

/** @brief Legacy alias for multiplayer_session_get_phase(). @return Current match phase. */
multiplayer_match_phase_t multiplayer_session_get_match_phase(void);

/** @brief Get countdown rounded to seconds. @return Countdown in seconds. */
uint8_t multiplayer_session_get_countdown_seconds(void);

/** @brief Get selected map identifier. @return Current map. */
multiplayer_map_t multiplayer_session_get_map(void);

/** @brief Check whether lobby conditions allow starting. @return true if start is allowed. */
bool multiplayer_session_can_start(void);

/** @brief Convert a game mode to text. @param mode Game mode. @return Constant string. */
const char *multiplayer_session_game_mode_to_string(multiplayer_game_mode_t mode);

/** @brief Convert a map to text. @param map Map identifier. @return Constant string. */
const char *multiplayer_session_map_to_string(multiplayer_map_t map);

/** @brief Convert a match phase to text. @param phase Match phase. @return Constant string. */
const char *multiplayer_session_match_phase_to_string(multiplayer_match_phase_t phase);

/** @brief Legacy getter for the first food item. @return Pointer to food. */
const food_t *multiplayer_session_get_food(void);

/** @brief Legacy getter for the first active powerup-like item. @return Pointer to food-compatible item. */
const food_t *multiplayer_session_get_powerup(void);

/** @brief Legacy alias to select the next game mode. */
void multiplayer_session_select_next_game_mode(void);

/** @brief Legacy map selection hook. */
void multiplayer_session_select_next_map(void);

/** @brief Legacy alias to request match start. */
void multiplayer_session_request_start(void);

#endif
