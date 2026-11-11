#ifndef SINGLEPLAYER_STATE_H
#define SINGLEPLAYER_STATE_H

#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>

#include "food.h"
#include "powerup.h"
#include "score.h"
#include "snake.h"

/**
 * @file singleplayer_state.h
 * @brief State and event API for the local singleplayer match.
 *
 * Keeps the authoritative singleplayer game model: snake, food, score,
 * powerups, active effects, animation data and death/game-over flow.
 */

#define SINGLEPLAYER_GRID_COLUMNS 56
#define SINGLEPLAYER_GRID_ROWS 38
#define SINGLEPLAYER_CELL_SIZE 16
#define SINGLEPLAYER_MOVE_TICKS 8
#define SINGLEPLAYER_DEATH_IMPACT_TICKS 35
#define SINGLEPLAYER_DEATH_ANIMATION_TICKS 95
#define SINGLEPLAYER_POWERUP_SPAWN_TICKS 300

#define SINGLEPLAYER_EFFECT_SLOTS 2
#define SINGLEPLAYER_DOUBLE_POINTS_TICKS 600
#define SINGLEPLAYER_GHOST_TICKS 360
#define SINGLEPLAYER_MAGNET_TICKS 480
#define SINGLEPLAYER_FRENZY_TICKS 360
#define SINGLEPLAYER_BONUS_FOOD_TICKS 480

#define SINGLEPLAYER_FRENZY_FOOD_COUNT 2
#define SINGLEPLAYER_BONUS_FOOD_POINTS 25

/**
 * @brief Reset all singleplayer entities and timers to a new match.
 *
 * @note Clears held powerups, queued powerups, effects and death animation.
 */
void singleplayer_state_reset(void);

/**
 * @brief Check whether the singleplayer model has been initialized.
 *
 * @return true if ready, false otherwise.
 */
bool singleplayer_state_is_ready(void);

/**
 * @brief Get the current player snake.
 *
 * @return Pointer to the snake, or NULL if the state is not ready.
 */
const snake_t *singleplayer_state_get_snake(void);

/**
 * @brief Get the main food item.
 *
 * @return Pointer to the food, or NULL if the state is not ready.
 */
const food_t *singleplayer_state_get_food(void);

/**
 * @brief Get how many positions describe the current food animation path.
 *
 * @return Number of stored animation positions.
 */
uint8_t singleplayer_state_get_food_animation_position_count(void);

/**
 * @brief Get a food animation position by index.
 *
 * @param index Position index.
 * @return Stored position; out-of-range indexes clamp to the last position.
 */
position_t singleplayer_state_get_food_animation_position(uint8_t index);

/**
 * @brief Get the currently spawned powerup item.
 *
 * @return Pointer to the powerup, or NULL if the state is not ready.
 */
const powerup_t *singleplayer_state_get_powerup(void);

/**
 * @brief Get the live score.
 *
 * @return Pointer to the score, or NULL if the state is not ready.
 */
const score_t *singleplayer_state_get_score(void);

/**
 * @brief Get the score captured when the player died.
 *
 * @return Pointer to the final score.
 */
const score_t *singleplayer_state_get_final_score(void);

/**
 * @brief Get the snake length before the latest movement step.
 *
 * @return Previous snake length.
 */
uint16_t singleplayer_state_get_previous_snake_length(void);

/**
 * @brief Get a snake segment from the previous movement step.
 *
 * @param index Segment index.
 * @return Previous segment position, or {0,0} when out of range.
 */
position_t singleplayer_state_get_previous_snake_segment(uint16_t index);

/**
 * @brief Get the current interpolation step inside the movement tick window.
 *
 * @return Current animation step.
 */
uint8_t singleplayer_state_get_animation_step(void);

/**
 * @brief Get the number of steps used for movement interpolation.
 *
 * @return Number of animation steps.
 */
uint8_t singleplayer_state_get_animation_steps(void);

/**
 * @brief Check whether the death animation is running.
 *
 * @return true while death animation is active.
 */
bool singleplayer_state_is_death_animating(void);

/**
 * @brief Get the current death animation tick.
 *
 * @return Current death animation tick.
 */
uint16_t singleplayer_state_get_death_animation_tick(void);

/**
 * @brief Get the full death animation duration.
 *
 * @return Duration in timer ticks.
 */
uint16_t singleplayer_state_get_death_animation_ticks(void);

/**
 * @brief Get the number of active timed effects.
 *
 * @return Number of active effect slots.
 */
uint8_t singleplayer_state_get_effect_count(void);

/**
 * @brief Get the type of an active timed effect.
 *
 * @param index Active effect index.
 * @return Effect type, or POWERUP_DOUBLE_POINTS as a safe default.
 */
powerup_type_t singleplayer_state_get_effect_type(uint8_t index);

/**
 * @brief Get the remaining ticks of an active timed effect.
 *
 * @param index Active effect index.
 * @return Remaining ticks, or 0 when out of range.
 */
uint16_t singleplayer_state_get_effect_ticks(uint8_t index);

/**
 * @brief Check whether a shield is armed.
 *
 * @return true if a fatal collision can be absorbed.
 */
bool singleplayer_state_has_shield(void);

/**
 * @brief Check whether bonus-food scoring is active.
 *
 * @return true while the bonus-food effect is active.
 */
bool singleplayer_state_is_bonus_food_active(void);

/**
 * @brief Check whether the player has a held powerup.
 *
 * @return true if a powerup can be activated.
 */
bool singleplayer_state_has_held_powerup(void);

/**
 * @brief Get the currently held powerup.
 *
 * @return Held powerup type.
 */
powerup_type_t singleplayer_state_get_held_powerup(void);

/**
 * @brief Check whether a second powerup is queued.
 *
 * @return true if the held slot will be refilled after activation.
 */
bool singleplayer_state_has_queued_powerup(void);

/**
 * @brief Get the queued powerup.
 *
 * @return Queued powerup type.
 */
powerup_type_t singleplayer_state_get_queued_powerup(void);

/**
 * @brief Get the extra food field used by frenzy.
 *
 * @return Pointer to the frenzy field, or NULL if the state is not ready.
 */
const food_field_t *singleplayer_state_get_frenzy_field(void);

/**
 * @brief Advance the singleplayer simulation by one timer tick.
 *
 * @note May move the snake, spawn powerups, update effects, update score or
 * transition to STATE_GAME_OVER after death animation.
 */
void singleplayer_state_handle_timer(void);

/**
 * @brief Handle keyboard controls for movement, pause and powerup activation.
 *
 * @param scancode Keyboard scancode.
 */
void singleplayer_state_handle_keyboard(uint8_t scancode);

/**
 * @brief Handle mouse controls for held powerup activation.
 *
 * @param packet Latest mouse packet.
 */
void singleplayer_state_handle_mouse(const struct packet *packet);

#endif
