/**
 * @file multiplayer_session_rules.c
 * @brief Host-authoritative multiplayer rules, movement, items and collisions.
 */

#include "multiplayer_session_internal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "timer.h"
#include "magnet.h"
#include "multiplayer_collision.h"
#include "multiplayer_effects.h"

static const powerup_type_t multiplayer_powerup_types[] = {
  POWERUP_DOUBLE_POINTS,
  POWERUP_GHOST,
  POWERUP_SHIELD,
  POWERUP_MAGNET,
  POWERUP_FRENZY,
  POWERUP_BONUS_FOOD,
  POWERUP_SPEED_BOOST,
  POWERUP_SLOW_ENEMY,
  POWERUP_FORCE_GROWTH
};

typedef struct {
  const food_t *moving_food;
  uint8_t food_index;
} multiplayer_magnet_context_t;

static uint8_t multiplayer_session_player_index(uint8_t player_id) {
  return player_id == MP_PLAYER_2 ? 1 : 0;
}

static snake_t *multiplayer_session_player_snake_mut(uint8_t index) {
  return index == 0 ? &host_player1 : &host_player2;
}

static snake_t *multiplayer_session_enemy_snake_mut(uint8_t index) {
  return index == 0 ? &host_player2 : &host_player1;
}

static uint16_t *multiplayer_session_player_score_mut(uint8_t index) {
  return index == 0 ? &player1_score : &player2_score;
}

static bool multiplayer_session_magnet_position_is_blocked(position_t position, position_t target, const void *context) {
  const multiplayer_magnet_context_t *magnet_context = (const multiplayer_magnet_context_t *) context;
  const food_t *moving_food = magnet_context != NULL ? magnet_context->moving_food : NULL;
  uint16_t grid_columns = multiplayer_session_active_grid_columns();
  uint16_t grid_rows = multiplayer_session_active_grid_rows();

  if (multiplayer_collision_cell_is_solid(position, current_game_mode, grid_columns, grid_rows))
    return true;

  if (multiplayer_collision_positions_equal(position, target))
    return false;

  if (multiplayer_collision_position_on_snake(position, &host_player1) ||
      multiplayer_collision_position_on_snake(position, &host_player2))
    return true;

  for (uint8_t i = 0; i < 2; i++) {
    if (&host_score_food[i] != moving_food &&
        food_is_active(&host_score_food[i]) &&
        multiplayer_collision_positions_equal(position, food_get_position(&host_score_food[i])))
      return true;

    if (powerup_is_active(&host_player_powerups[i]) &&
        multiplayer_collision_positions_equal(position, powerup_get_position(&host_player_powerups[i])))
      return true;
  }

  return false;
}

static void multiplayer_session_record_magnet_food_position(position_t position, void *context) {
  multiplayer_magnet_context_t *magnet_context = (multiplayer_magnet_context_t *) context;

  if (magnet_context == NULL)
    return;

  multiplayer_session_append_host_food_animation_position(magnet_context->food_index, position);
}

static void multiplayer_session_apply_magnet_to_food(uint8_t food_index, const snake_t *snake) {
  if (food_index >= 2 || snake == NULL || !food_is_active(&host_score_food[food_index]) || !snake_is_alive(snake))
    return;

  multiplayer_magnet_context_t context = {
    .moving_food = &host_score_food[food_index],
    .food_index = food_index
  };

  (void) magnet_pull_food_towards(&host_score_food[food_index],
                                  snake_get_head(snake),
                                  multiplayer_session_magnet_position_is_blocked,
                                  &context,
                                  multiplayer_session_record_magnet_food_position,
                                  &context);
}

static void multiplayer_session_apply_magnet_effects(void) {
  for (uint8_t player = 0; player < 2; player++) {
    if (!multiplayer_effects_active(player, POWERUP_MAGNET))
      continue;

    const snake_t *snake = multiplayer_session_player_snake_mut(player);

    if (current_game_mode == MP_GAME_MODE_SCORE)
      multiplayer_session_apply_magnet_to_food(player, snake);
    else {
      for (uint8_t i = 0; i < MULTIPLAYER_ARENA_FOOD_COUNT && i < 2; i++)
        multiplayer_session_apply_magnet_to_food(i, snake);
    }
  }
}

static bool multiplayer_session_position_on_food_slot(position_t position, uint8_t skip_index) {
  for (uint8_t i = 0; i < 2; i++) {
    if (i == skip_index)
      continue;

    if (food_is_active(&host_score_food[i]) && multiplayer_collision_positions_equal(food_get_position(&host_score_food[i]), position))
      return true;
  }

  return false;
}

static bool multiplayer_session_position_on_powerup_slot(position_t position, uint8_t skip_index) {
  for (uint8_t i = 0; i < 2; i++) {
    if (i == skip_index)
      continue;

    if (powerup_is_active(&host_player_powerups[i]) && multiplayer_collision_positions_equal(powerup_get_position(&host_player_powerups[i]), position))
      return true;
  }

  return false;
}

uint16_t multiplayer_session_active_grid_columns(void) {
  return current_game_mode == MP_GAME_MODE_ARENA ? MULTIPLAYER_ARENA_GRID_COLUMNS : MULTIPLAYER_SCORE_GRID_COLUMNS;
}

uint16_t multiplayer_session_active_grid_rows(void) {
  return current_game_mode == MP_GAME_MODE_ARENA ? MULTIPLAYER_ARENA_GRID_ROWS : MULTIPLAYER_SCORE_GRID_ROWS;
}

static bool multiplayer_session_is_valid_spawn_position(position_t position, uint8_t food_skip, uint8_t powerup_skip) {
  uint16_t grid_columns = multiplayer_session_active_grid_columns();
  uint16_t grid_rows = multiplayer_session_active_grid_rows();

  if (multiplayer_collision_cell_is_solid(position, current_game_mode, grid_columns, grid_rows))
    return false;

  if (multiplayer_collision_position_on_snake(position, &host_player1) ||
      multiplayer_collision_position_on_snake(position, &host_player2) ||
      multiplayer_session_position_on_food_slot(position, food_skip) ||
      multiplayer_session_position_on_powerup_slot(position, powerup_skip))
    return false;

  return true;
}

static uint32_t multiplayer_session_spawn_seed(uint8_t slot, uint8_t salt) {
  uint32_t seed = timer_get_counter();
  seed ^= ((uint32_t) host_tick + 1u) * 1103u;
  seed ^= ((uint32_t) player1_score + 3u) * 1741u;
  seed ^= ((uint32_t) player2_score + 5u) * 2593u;
  seed ^= ((uint32_t) slot + 7u) * 7919u;
  seed ^= ((uint32_t) salt + 11u) * 3571u;
  return seed;
}

static bool multiplayer_session_spawn_food_slot(uint8_t index) {
  if (index >= 2)
    return false;

  uint16_t grid_columns = multiplayer_session_active_grid_columns();
  uint16_t grid_rows = multiplayer_session_active_grid_rows();
  uint32_t cell_count = (uint32_t) grid_columns * grid_rows;
  uint32_t start = multiplayer_session_spawn_seed(index, 1) % cell_count;

  food_init(&host_score_food[index], grid_columns, grid_rows);

  for (uint32_t offset = 0; offset < cell_count; offset++) {
    uint32_t cell = (start + offset) % cell_count;
    position_t position = {
      .x = (int16_t) (cell % grid_columns),
      .y = (int16_t) (cell / grid_columns)
    };

    if (multiplayer_session_is_valid_spawn_position(position, index, 2)) {
      host_score_food[index].position = position;
      host_score_food[index].active = true;
      multiplayer_session_reset_host_food_animation_slot(index);
      return true;
    }
  }

  host_score_food[index].active = false;
  multiplayer_session_reset_host_food_animation_slot(index);
  return false;
}

static bool multiplayer_session_spawn_powerup_slot(uint8_t index) {
  if (index >= 2)
    return false;

  uint16_t grid_columns = multiplayer_session_active_grid_columns();
  uint16_t grid_rows = multiplayer_session_active_grid_rows();
  uint32_t cell_count = (uint32_t) grid_columns * grid_rows;
  uint32_t seed = multiplayer_session_spawn_seed(index, 2);
  uint32_t start = seed % cell_count;
  uint8_t type_count = (uint8_t) (sizeof(multiplayer_powerup_types) / sizeof(multiplayer_powerup_types[0]));

  powerup_init(&host_player_powerups[index], grid_columns, grid_rows);

  for (uint32_t offset = 0; offset < cell_count; offset++) {
    uint32_t cell = (start + offset) % cell_count;
    position_t position = {
      .x = (int16_t) (cell % grid_columns),
      .y = (int16_t) (cell / grid_columns)
    };

    if (multiplayer_session_is_valid_spawn_position(position, 2, index)) {
      host_player_powerups[index].position = position;
      host_player_powerups[index].type = multiplayer_powerup_types[(seed + offset) % type_count];
      host_player_powerups[index].active = true;
      return true;
    }
  }

  host_player_powerups[index].active = false;
  return false;
}

static void multiplayer_session_sync_legacy_items(void) {
  host_food = host_score_food[0];

  food_init(&host_powerup, multiplayer_session_active_grid_columns(), multiplayer_session_active_grid_rows());
  host_powerup.active = false;

  for (uint8_t i = 0; i < 2; i++) {
    if (powerup_is_active(&host_player_powerups[i])) {
      host_powerup.position = powerup_get_position(&host_player_powerups[i]);
      host_powerup.active = true;
      return;
    }
  }
}

static void multiplayer_session_grow_snake_by(snake_t *snake, uint8_t amount) {
  if (snake == NULL)
    return;

  for (uint8_t i = 0; i < amount; i++) {
    if (snake_get_length(snake) >= MULTIPLAYER_MAX_SYNCED_SNAKE_SEGMENTS)
      return;

    snake_grow(snake);
  }
}

static void multiplayer_session_set_snake_at(snake_t *snake, int16_t head_x, int16_t head_y, direction_t direction) {
  if (snake == NULL)
    return;

  snake->current_direction = direction;
  snake->next_direction = direction;
  snake->alive = true;

  for (uint16_t i = 0; i < snake->length; i++) {
    snake->body[i].x = head_x;
    snake->body[i].y = head_y;

    if (direction == DIRECTION_RIGHT)
      snake->body[i].x = (int16_t) (head_x - (int16_t) i);
    else if (direction == DIRECTION_LEFT)
      snake->body[i].x = (int16_t) (head_x + (int16_t) i);
    else if (direction == DIRECTION_UP)
      snake->body[i].y = (int16_t) (head_y + (int16_t) i);
    else if (direction == DIRECTION_DOWN)
      snake->body[i].y = (int16_t) (head_y - (int16_t) i);
  }
}

static void multiplayer_session_position_score_snakes(void) {
  snake_init(&host_player1, MULTIPLAYER_SCORE_GRID_COLUMNS, MULTIPLAYER_SCORE_GRID_ROWS);
  snake_init(&host_player2, MULTIPLAYER_SCORE_GRID_COLUMNS, MULTIPLAYER_SCORE_GRID_ROWS);
  multiplayer_session_set_snake_at(&host_player1, (int16_t) (MULTIPLAYER_SCORE_GRID_COLUMNS / 2), (int16_t) (MULTIPLAYER_SCORE_GRID_ROWS / 2), DIRECTION_RIGHT);
  multiplayer_session_set_snake_at(&host_player2, (int16_t) (MULTIPLAYER_SCORE_GRID_COLUMNS / 2), (int16_t) (MULTIPLAYER_SCORE_GRID_ROWS / 2), DIRECTION_RIGHT);
}

static void multiplayer_session_position_arena_snakes(void) {
  snake_init(&host_player1, MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);
  snake_init(&host_player2, MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);
  multiplayer_session_set_snake_at(&host_player1, 10, 10, DIRECTION_RIGHT);
  multiplayer_session_set_snake_at(&host_player2, (int16_t) (MULTIPLAYER_ARENA_GRID_COLUMNS - 11), (int16_t) (MULTIPLAYER_ARENA_GRID_ROWS - 11), DIRECTION_LEFT);
}

static void multiplayer_session_clear_powerup_state(void) {
  for (uint8_t i = 0; i < 2; i++) {
    host_held_powerup_active[i] = false;
    host_held_powerup[i] = POWERUP_DOUBLE_POINTS;
    host_queued_powerup_active[i] = false;
    host_queued_powerup[i] = POWERUP_DOUBLE_POINTS;
    host_shield_active[i] = false;
    player_death_animation_tick[i] = 0;
  }

  multiplayer_effects_clear_all();
}

void multiplayer_session_prepare_match_entities(void) {
  player1_score = 0;
  player2_score = 0;
  winner = 0;
  movement_tick_counter = 0;
  countdown_tick_counter = 0;
  snapshot_tick_counter = 0;
  countdown_seconds = MULTIPLAYER_COUNTDOWN_SECONDS;
  host_tick = 0;
  last_snapshot_tick = 0;
  death_animation_tick = 0;
  multiplayer_session_clear_powerup_state();

  if (current_game_mode == MP_GAME_MODE_ARENA) {
    multiplayer_session_position_arena_snakes();
    food_init(&host_food, MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);
    food_init(&host_powerup, MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);

    for (uint8_t i = 0; i < 2; i++) {
      food_init(&host_score_food[i], MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);
      powerup_init(&host_player_powerups[i], MULTIPLAYER_ARENA_GRID_COLUMNS, MULTIPLAYER_ARENA_GRID_ROWS);
      (void) multiplayer_session_spawn_food_slot(i);
      (void) multiplayer_session_spawn_powerup_slot(i);
    }
  }
  else {
    multiplayer_session_position_score_snakes();
    food_init(&host_food, MULTIPLAYER_SCORE_GRID_COLUMNS, MULTIPLAYER_SCORE_GRID_ROWS);
    food_init(&host_powerup, MULTIPLAYER_SCORE_GRID_COLUMNS, MULTIPLAYER_SCORE_GRID_ROWS);

    for (uint8_t i = 0; i < 2; i++) {
      food_init(&host_score_food[i], MULTIPLAYER_SCORE_GRID_COLUMNS, MULTIPLAYER_SCORE_GRID_ROWS);
      powerup_init(&host_player_powerups[i], MULTIPLAYER_SCORE_GRID_COLUMNS, MULTIPLAYER_SCORE_GRID_ROWS);
      (void) multiplayer_session_spawn_food_slot(i);
      (void) multiplayer_session_spawn_powerup_slot(i);
    }
  }

  multiplayer_session_sync_legacy_items();
  multiplayer_session_store_previous_snakes();
  snapshot_player1 = host_player1;
  snapshot_player2 = host_player2;
  snapshot_food = host_score_food[0];
  snapshot_powerup = host_powerup;

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

  snapshot_available = true;
}

static void multiplayer_session_finish_match(uint8_t new_winner) {
  if (match_phase == MP_MATCH_FINISHED)
    return;

  winner = new_winner;
  death_animation_tick = 0;
  match_phase = MP_MATCH_FINISHED;
}

static uint8_t multiplayer_session_winner_by_score_or_tie(void) {
  if (player1_score > player2_score)
    return MP_PLAYER_1;

  if (player2_score > player1_score)
    return MP_PLAYER_2;

  return MP_WINNER_TIE;
}

void multiplayer_session_activate_player_powerup(uint8_t player_id) {
  uint8_t player_index = multiplayer_session_player_index(player_id);
  uint8_t enemy_index = player_index == 0 ? 1 : 0;
  snake_t *snake = multiplayer_session_player_snake_mut(player_index);
  snake_t *enemy = multiplayer_session_enemy_snake_mut(player_index);
  uint16_t *score = multiplayer_session_player_score_mut(player_index);

  if (!host_held_powerup_active[player_index] || snake == NULL || !snake_is_alive(snake))
    return;

  powerup_type_t type = host_held_powerup[player_index];

  if (host_queued_powerup_active[player_index]) {
    host_held_powerup[player_index] = host_queued_powerup[player_index];
    host_held_powerup_active[player_index] = true;
    host_queued_powerup_active[player_index] = false;
  }
  else
    host_held_powerup_active[player_index] = false;

  switch (type) {
    case POWERUP_DOUBLE_POINTS:
    case POWERUP_SPEED_BOOST:
    case POWERUP_GHOST:
    case POWERUP_MAGNET:
    case POWERUP_FRENZY:
      multiplayer_effects_add(player_index, type);
      break;

    case POWERUP_BONUS_FOOD:
      multiplayer_session_grow_snake_by(snake, 1);
      *score = (uint16_t) (*score + MULTIPLAYER_BONUS_FOOD_POINTS);
      break;

    case POWERUP_SLOW_ENEMY:
      multiplayer_effects_add(enemy_index, POWERUP_SLOW_ENEMY);
      break;

    case POWERUP_SHIELD:
      host_shield_active[player_index] = true;
      break;

    case POWERUP_FORCE_GROWTH:
      multiplayer_session_grow_snake_by(enemy, MULTIPLAYER_ARENA_POWERUP_GROWTH);
      *score = (uint16_t) (*score + 2);
      break;

    default:
      break;
  }
}

static void multiplayer_session_collect_powerup(uint8_t player_index, uint8_t powerup_index) {
  if (player_index >= 2 || powerup_index >= 2 || !powerup_is_active(&host_player_powerups[powerup_index]))
    return;

  powerup_type_t collected = powerup_get_type(&host_player_powerups[powerup_index]);

  if (!host_held_powerup_active[player_index]) {
    host_held_powerup[player_index] = collected;
    host_held_powerup_active[player_index] = true;
  }
  else {
    host_queued_powerup[player_index] = collected;
    host_queued_powerup_active[player_index] = true;
  }

  powerup_clear(&host_player_powerups[powerup_index]);
  (void) multiplayer_session_spawn_powerup_slot(powerup_index);
}

static bool multiplayer_session_consume_shield(uint8_t player_index) {
  if (player_index >= 2 || !host_shield_active[player_index])
    return false;

  host_shield_active[player_index] = false;
  return true;
}

static void multiplayer_session_kill_player(uint8_t player_index) {
  snake_t *snake = multiplayer_session_player_snake_mut(player_index);

  if (snake == NULL || !snake_is_alive(snake))
    return;

  if (multiplayer_session_consume_shield(player_index))
    return;

  snake_set_alive(snake, false);
  player_death_animation_tick[player_index] = 0;
}

static void multiplayer_session_advance_death_animations(void) {
  if (!snake_is_alive(&host_player1) && player_death_animation_tick[0] < MULTIPLAYER_DEATH_ANIMATION_TICKS)
    player_death_animation_tick[0]++;

  if (!snake_is_alive(&host_player2) && player_death_animation_tick[1] < MULTIPLAYER_DEATH_ANIMATION_TICKS)
    player_death_animation_tick[1]++;

  death_animation_tick = player_death_animation_tick[0] > player_death_animation_tick[1] ? player_death_animation_tick[0] : player_death_animation_tick[1];
}

static bool multiplayer_session_should_skip_move(uint8_t player_index) {
  if (!multiplayer_effects_active(player_index, POWERUP_SLOW_ENEMY))
    return false;

  return (host_tick & 1u) == 0;
}

static uint8_t multiplayer_session_move_repetitions(uint8_t player_index) {
  if (multiplayer_effects_active(player_index, POWERUP_SPEED_BOOST) ||
      multiplayer_effects_active(player_index, POWERUP_FRENZY))
    return 2;

  return 1;
}

static void multiplayer_session_apply_items_for_player_after_step(uint8_t player) {
  snake_t *snake = multiplayer_session_player_snake_mut(player);
  uint16_t *score = multiplayer_session_player_score_mut(player);

  if (snake == NULL || score == NULL || !snake_is_alive(snake))
    return;

  if (current_game_mode == MP_GAME_MODE_ARENA) {
    for (uint8_t food = 0; food < MULTIPLAYER_ARENA_FOOD_COUNT && food < 2; food++) {
      if (food_is_active(&host_score_food[food]) && food_collides_with_snake_head(&host_score_food[food], snake)) {
        multiplayer_session_grow_snake_by(snake, 1);
        *score = (uint16_t) (*score + (multiplayer_effects_active(player, POWERUP_DOUBLE_POINTS) ? 2 : 1) + (multiplayer_effects_active(player, POWERUP_FRENZY) ? 1 : 0));
        (void) multiplayer_session_spawn_food_slot(food);
      }
    }

    for (uint8_t powerup = 0; powerup < 2; powerup++) {
      if (powerup_is_active(&host_player_powerups[powerup]) && powerup_collides_with_snake_head(&host_player_powerups[powerup], snake))
        multiplayer_session_collect_powerup(player, powerup);
    }

    return;
  }

  if (food_is_active(&host_score_food[player]) && food_collides_with_snake_head(&host_score_food[player], snake)) {
    multiplayer_session_grow_snake_by(snake, 1);
    *score = (uint16_t) (*score + (multiplayer_effects_active(player, POWERUP_DOUBLE_POINTS) ? 2 : 1) + (multiplayer_effects_active(player, POWERUP_FRENZY) ? 1 : 0));
    (void) multiplayer_session_spawn_food_slot(player);
  }

  if (powerup_is_active(&host_player_powerups[player]) && powerup_collides_with_snake_head(&host_player_powerups[player], snake))
    multiplayer_session_collect_powerup(player, player);
}

static void multiplayer_session_move_player(uint8_t player_index) {
  snake_t *snake = multiplayer_session_player_snake_mut(player_index);
  uint16_t grid_columns = multiplayer_session_active_grid_columns();
  uint16_t grid_rows = multiplayer_session_active_grid_rows();

  if (snake == NULL || !snake_is_alive(snake))
    return;

  if (multiplayer_session_should_skip_move(player_index))
    return;

  uint8_t repetitions = multiplayer_session_move_repetitions(player_index);

  for (uint8_t i = 0; i < repetitions && snake_is_alive(snake); i++) {
    bool ghost = multiplayer_effects_active(player_index, POWERUP_GHOST);

    if (multiplayer_collision_next_head_hits_solid(snake, current_game_mode, grid_columns, grid_rows)) {
      multiplayer_session_kill_player(player_index);
      return;
    }

    if (ghost)
      snake_move_ignoring_self(snake);
    else
      snake_move(snake);

    if (!snake_is_alive(snake)) {
      if (multiplayer_session_consume_shield(player_index))
        snake_set_alive(snake, true);
      else {
        player_death_animation_tick[player_index] = 0;
        return;
      }
    }

    multiplayer_session_apply_items_for_player_after_step(player_index);
  }
}

static void multiplayer_session_apply_score_items_after_move(void) {
  for (uint8_t player = 0; player < 2; player++)
    multiplayer_session_apply_items_for_player_after_step(player);
}

static void multiplayer_session_apply_arena_items_after_move(void) {
  for (uint8_t player = 0; player < 2; player++)
    multiplayer_session_apply_items_for_player_after_step(player);
}


static void multiplayer_session_check_arena_collisions(void) {
  bool p1_dead = !snake_is_alive(&host_player1);
  bool p2_dead = !snake_is_alive(&host_player2);
  position_t p1_head = snake_get_head(&host_player1);
  position_t p2_head = snake_get_head(&host_player2);
  bool p1_ghost = multiplayer_effects_active(0, POWERUP_GHOST);
  bool p2_ghost = multiplayer_effects_active(1, POWERUP_GHOST);

  if (!p1_dead && !p2_dead && !p1_ghost && !p2_ghost && multiplayer_collision_positions_equal(p1_head, p2_head)) {
    multiplayer_session_kill_player(0);
    multiplayer_session_kill_player(1);
  }

  p1_dead = !snake_is_alive(&host_player1);
  p2_dead = !snake_is_alive(&host_player2);

  if (!p1_dead && !p1_ghost && multiplayer_collision_position_on_snake(p1_head, &host_player2))
    multiplayer_session_kill_player(0);

  if (!p2_dead && !p2_ghost && multiplayer_collision_position_on_snake(p2_head, &host_player1))
    multiplayer_session_kill_player(1);

  p1_dead = !snake_is_alive(&host_player1);
  p2_dead = !snake_is_alive(&host_player2);

  if (p1_dead && p2_dead)
    multiplayer_session_finish_match(multiplayer_session_winner_by_score_or_tie());
  else if (p1_dead)
    multiplayer_session_finish_match(MP_PLAYER_2);
  else if (p2_dead)
    multiplayer_session_finish_match(MP_PLAYER_1);
}

static void multiplayer_session_update_score_mode_after_move(void) {
  multiplayer_session_apply_score_items_after_move();

  bool p1_dead = !snake_is_alive(&host_player1);
  bool p2_dead = !snake_is_alive(&host_player2);

  if (p1_dead && p2_dead)
    multiplayer_session_finish_match(multiplayer_session_winner_by_score_or_tie());
  else if (p1_dead && player2_score > player1_score)
    multiplayer_session_finish_match(MP_PLAYER_2);
  else if (p2_dead && player1_score > player2_score)
    multiplayer_session_finish_match(MP_PLAYER_1);
}

void multiplayer_session_handle_timer(void) {
  if (current_role == MP_ROLE_CLIENT) {
    if (match_phase == MP_MATCH_PLAYING && movement_tick_counter < MULTIPLAYER_MOVE_TICKS - 1)
      movement_tick_counter++;

    if ((match_phase == MP_MATCH_PLAYING || match_phase == MP_MATCH_FINISHED) && snapshot_available) {
      if (!snake_is_alive(&snapshot_player1) && player_death_animation_tick[0] < MULTIPLAYER_DEATH_ANIMATION_TICKS)
        player_death_animation_tick[0]++;

      if (!snake_is_alive(&snapshot_player2) && player_death_animation_tick[1] < MULTIPLAYER_DEATH_ANIMATION_TICKS)
        player_death_animation_tick[1]++;

      death_animation_tick = player_death_animation_tick[0] > player_death_animation_tick[1] ? player_death_animation_tick[0] : player_death_animation_tick[1];
    }
    else if (match_phase == MP_MATCH_FINISHED && death_animation_tick < MULTIPLAYER_DEATH_ANIMATION_TICKS)
      death_animation_tick++;

    return;
  }

  if (current_role != MP_ROLE_HOST || match_phase == MP_MATCH_LOBBY)
    return;

  snapshot_tick_counter++;

  if (match_phase == MP_MATCH_COUNTDOWN) {
    countdown_tick_counter++;

    if (countdown_tick_counter >= MP_COUNTDOWN_TICKS_PER_SECOND) {
      countdown_tick_counter = 0;

      if (countdown_seconds > 0)
        countdown_seconds--;

      if (countdown_seconds == 0)
        match_phase = MP_MATCH_PLAYING;
    }

    if (snapshot_tick_counter >= MP_SNAPSHOT_PERIOD_TICKS) {
      snapshot_tick_counter = 0;
      multiplayer_session_send_snapshot();
    }

    return;
  }

  multiplayer_session_advance_death_animations();

  if (match_phase == MP_MATCH_FINISHED) {
    if (snapshot_tick_counter >= MP_SNAPSHOT_PERIOD_TICKS) {
      snapshot_tick_counter = 0;
      multiplayer_session_send_snapshot();
    }

    return;
  }

  if (match_phase != MP_MATCH_PLAYING)
    return;

  multiplayer_effects_tick_all();
  movement_tick_counter++;

  if (movement_tick_counter < MULTIPLAYER_MOVE_TICKS) {
    if (snapshot_tick_counter >= MP_SNAPSHOT_PERIOD_TICKS) {
      snapshot_tick_counter = 0;
      multiplayer_session_sync_legacy_items();
      multiplayer_session_send_snapshot();
    }

    return;
  }

  movement_tick_counter = 0;
  host_tick++;
  multiplayer_session_store_previous_snakes();
  multiplayer_session_store_host_food_animation_starts();

  multiplayer_session_move_player(0);
  multiplayer_session_move_player(1);
  multiplayer_session_apply_magnet_effects();

  if (current_game_mode == MP_GAME_MODE_ARENA) {
    multiplayer_session_apply_arena_items_after_move();
    multiplayer_session_check_arena_collisions();
  }
  else
    multiplayer_session_update_score_mode_after_move();

  multiplayer_session_sync_legacy_items();
  snapshot_tick_counter = 0;
  multiplayer_session_send_snapshot();
}
