/**
 * @file singleplayer_state.c
 * @brief Authoritative singleplayer simulation and event handling.
 */

#include "singleplayer_state.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "collision.h"
#include "high_score.h"
#include "magnet.h"
#include "mouse_cursor.h"
#include "state.h"

static const powerup_type_t singleplayer_powerup_types[] = {
  POWERUP_DOUBLE_POINTS,
  POWERUP_SHIELD,
  POWERUP_MAGNET,
  POWERUP_FRENZY
};

typedef struct {
  powerup_type_t type;
  uint16_t ticks;
  bool active;
} singleplayer_effect_t;

static snake_t player_snake;
static food_t player_food;
static powerup_t player_powerup;
static score_t player_score;
static score_t final_score;
static position_t food_animation_positions[3];
static uint8_t food_animation_position_count = 0;
static position_t previous_body[SNAKE_MAX_SEGMENTS];
static uint16_t previous_length = 0;
static bool ready = false;
static bool death_animating = false;
static uint16_t death_animation_tick = 0;
static uint8_t movement_tick_counter = 0;
static uint16_t powerup_spawn_timer = 0;
static powerup_type_t held_powerup = POWERUP_DOUBLE_POINTS;
static bool has_held_powerup = false;
static powerup_type_t queued_powerup = POWERUP_DOUBLE_POINTS;
static bool has_queued_powerup = false;

static singleplayer_effect_t effects[SINGLEPLAYER_EFFECT_SLOTS];
static bool shield_charged = false;

static food_field_t frenzy_field;
static bool frenzy_active_prev = false;

static void singleplayer_state_ensure_ready(void) {
  if (!ready)
    singleplayer_state_reset();
}

static void singleplayer_state_finish_game(void) {
  state_set_current(STATE_GAME_OVER);
}

static void singleplayer_state_start_death_animation(void) {
  final_score = player_score;
  (void) high_score_submit(score_get_value(&player_score));
  death_animating = true;
  death_animation_tick = 0;
  movement_tick_counter = 0;
}

static void singleplayer_state_store_previous_snake(void) {
  previous_length = snake_get_length(&player_snake);

  for (uint16_t i = 0; i < previous_length; i++)
    previous_body[i] = snake_get_segment(&player_snake, i);
}

static bool singleplayer_position_equals(position_t first, position_t second) {
  return first.x == second.x && first.y == second.y;
}

static void singleplayer_state_store_previous_food(void) {
  food_animation_positions[0] = food_get_position(&player_food);
  food_animation_position_count = 1;
}

static void singleplayer_state_append_food_animation_position(void) {
  position_t position = food_get_position(&player_food);

  if (food_animation_position_count == 0) {
    food_animation_positions[0] = position;
    food_animation_position_count = 1;
    return;
  }

  if (singleplayer_position_equals(food_animation_positions[food_animation_position_count - 1], position))
    return;

  if (food_animation_position_count < (uint8_t) (sizeof(food_animation_positions) / sizeof(food_animation_positions[0]))) {
    food_animation_positions[food_animation_position_count] = position;
    food_animation_position_count++;
    return;
  }

  food_animation_positions[food_animation_position_count - 1] = position;
}

static uint16_t singleplayer_effect_duration(powerup_type_t type) {
  switch (type) {
    case POWERUP_DOUBLE_POINTS:
      return SINGLEPLAYER_DOUBLE_POINTS_TICKS;

    case POWERUP_GHOST:
      return SINGLEPLAYER_GHOST_TICKS;

    case POWERUP_MAGNET:
      return SINGLEPLAYER_MAGNET_TICKS;

    case POWERUP_FRENZY:
      return SINGLEPLAYER_FRENZY_TICKS;

    case POWERUP_BONUS_FOOD:
      return SINGLEPLAYER_BONUS_FOOD_TICKS;

    default:
      return 0;
  }
}

static bool singleplayer_effect_active(powerup_type_t type) {
  for (uint8_t i = 0; i < SINGLEPLAYER_EFFECT_SLOTS; i++) {
    if (effects[i].active && effects[i].type == type)
      return true;
  }

  return false;
}

static void singleplayer_effect_add(powerup_type_t type) {
  uint16_t duration = singleplayer_effect_duration(type);

  if (duration == 0)
    return;

  for (uint8_t i = 0; i < SINGLEPLAYER_EFFECT_SLOTS; i++) {
    if (effects[i].active && effects[i].type == type) {
      effects[i].ticks = duration;
      return;
    }
  }

  for (uint8_t i = 0; i < SINGLEPLAYER_EFFECT_SLOTS; i++) {
    if (!effects[i].active) {
      effects[i].type = type;
      effects[i].ticks = duration;
      effects[i].active = true;
      return;
    }
  }

  for (uint8_t i = 1; i < SINGLEPLAYER_EFFECT_SLOTS; i++)
    effects[i - 1] = effects[i];

  effects[SINGLEPLAYER_EFFECT_SLOTS - 1].type = type;
  effects[SINGLEPLAYER_EFFECT_SLOTS - 1].ticks = duration;
  effects[SINGLEPLAYER_EFFECT_SLOTS - 1].active = true;
}

static void singleplayer_state_activate_powerup(powerup_type_t type) {
  switch (type) {
    case POWERUP_SHIELD:
      shield_charged = true;
      break;

    case POWERUP_BONUS_FOOD:
      score_add_points(&player_score, SINGLEPLAYER_BONUS_FOOD_POINTS);
      break;

    case POWERUP_DOUBLE_POINTS:
    case POWERUP_MAGNET:
    case POWERUP_FRENZY:
      singleplayer_effect_add(type);
      break;

    default:
      break;
  }
}

static void singleplayer_state_activate_held_powerup(void) {
  if (!has_held_powerup)
    return;

  if (!snake_is_alive(&player_snake))
    return;

  powerup_type_t type = held_powerup;

  if (has_queued_powerup) {
    held_powerup = queued_powerup;
    has_held_powerup = true;
    has_queued_powerup = false;
  }
  else
    has_held_powerup = false;

  singleplayer_state_activate_powerup(type);
}

static void singleplayer_effects_tick(void) {
  for (uint8_t i = 0; i < SINGLEPLAYER_EFFECT_SLOTS; i++) {
    if (effects[i].active && effects[i].ticks > 0) {
      effects[i].ticks--;

      if (effects[i].ticks == 0)
        effects[i].active = false;
    }
  }

  for (uint8_t i = 1; i < SINGLEPLAYER_EFFECT_SLOTS; i++) {
    if (!effects[i - 1].active && effects[i].active) {
      effects[i - 1] = effects[i];
      effects[i].active = false;
    }
  }

  score_set_multiplier(&player_score, singleplayer_effect_active(POWERUP_DOUBLE_POINTS) ? 2 : 1);
}

static void singleplayer_update_frenzy_field(void) {
  bool frenzy = singleplayer_effect_active(POWERUP_FRENZY);

  if (frenzy && !frenzy_active_prev)
    food_field_spawn_all(&frenzy_field, &player_snake, &player_food);
  else if (!frenzy && frenzy_active_prev)
    food_field_clear(&frenzy_field);

  frenzy_active_prev = frenzy;
}

static bool singleplayer_magnet_position_is_blocked(position_t position, position_t target, const void *context) {
  (void) context;

  if (position.x < 0 || position.y < 0 ||
      position.x >= SINGLEPLAYER_GRID_COLUMNS ||
      position.y >= SINGLEPLAYER_GRID_ROWS)
    return true;

  if (singleplayer_position_equals(position, target))
    return false;

  uint16_t length = snake_get_length(&player_snake);

  for (uint16_t i = 1; i < length; i++) {
    if (singleplayer_position_equals(position, snake_get_segment(&player_snake, i)))
      return true;
  }

  return false;
}

static void singleplayer_state_record_magnet_food_position(position_t position, void *context) {
  (void) position;
  (void) context;
  singleplayer_state_append_food_animation_position();
}

static void singleplayer_state_apply_magnet(void) {
  if (!food_is_active(&player_food))
    return;

  (void) magnet_pull_food_towards(&player_food,
                                  snake_get_head(&player_snake),
                                  singleplayer_magnet_position_is_blocked,
                                  NULL,
                                  singleplayer_state_record_magnet_food_position,
                                  NULL);
}

static bool singleplayer_state_shield_escape(void) {
  direction_t dirs[6] = {
    snake_get_next_direction(&player_snake),
    snake_get_current_direction(&player_snake),
    DIRECTION_UP,
    DIRECTION_RIGHT,
    DIRECTION_DOWN,
    DIRECTION_LEFT
  };
  bool tried[4] = {false, false, false, false};

  for (uint8_t i = 0; i < 6; i++) {
    direction_t candidate = dirs[i];

    if (candidate < DIRECTION_UP || candidate > DIRECTION_LEFT || tried[candidate])
      continue;

    tried[candidate] = true;

    if (!snake_direction_would_be_fatal(&player_snake, candidate)) {
      player_snake.next_direction = candidate;
      snake_move(&player_snake);
      return snake_is_alive(&player_snake);
    }
  }

  return false;
}

void singleplayer_state_reset(void) {
  snake_init(&player_snake, SINGLEPLAYER_GRID_COLUMNS, SINGLEPLAYER_GRID_ROWS);
  food_init(&player_food, SINGLEPLAYER_GRID_COLUMNS, SINGLEPLAYER_GRID_ROWS);
  food_spawn(&player_food, &player_snake);
  food_field_init(&frenzy_field, SINGLEPLAYER_FRENZY_FOOD_COUNT, SINGLEPLAYER_GRID_COLUMNS, SINGLEPLAYER_GRID_ROWS);
  powerup_init(&player_powerup, SINGLEPLAYER_GRID_COLUMNS, SINGLEPLAYER_GRID_ROWS);
  score_reset(&player_score);
  score_reset(&final_score);
  singleplayer_state_store_previous_snake();
  singleplayer_state_store_previous_food();
  death_animating = false;
  death_animation_tick = 0;
  movement_tick_counter = 0;
  powerup_spawn_timer = 0;
  held_powerup = POWERUP_DOUBLE_POINTS;
  has_held_powerup = false;
  queued_powerup = POWERUP_DOUBLE_POINTS;
  has_queued_powerup = false;

  for (uint8_t i = 0; i < SINGLEPLAYER_EFFECT_SLOTS; i++) {
    effects[i].active = false;
    effects[i].ticks = 0;
  }

  shield_charged = false;
  frenzy_active_prev = false;
  ready = true;
}

bool singleplayer_state_is_ready(void) {
  return ready;
}

const snake_t *singleplayer_state_get_snake(void) {
  if (!ready)
    return NULL;

  return &player_snake;
}

const food_t *singleplayer_state_get_food(void) {
  if (!ready)
    return NULL;

  return &player_food;
}

uint8_t singleplayer_state_get_food_animation_position_count(void) {
  if (!ready)
    return 0;

  return food_animation_position_count;
}

position_t singleplayer_state_get_food_animation_position(uint8_t index) {
  position_t empty = {0, 0};

  if (!ready)
    return empty;

  if (food_animation_position_count == 0)
    return food_get_position(&player_food);

  if (index >= food_animation_position_count)
    index = food_animation_position_count - 1;

  return food_animation_positions[index];
}

const powerup_t *singleplayer_state_get_powerup(void) {
  if (!ready)
    return NULL;

  return &player_powerup;
}

const score_t *singleplayer_state_get_score(void) {
  if (!ready)
    return NULL;

  return &player_score;
}

const score_t *singleplayer_state_get_final_score(void) {
  return &final_score;
}

uint16_t singleplayer_state_get_previous_snake_length(void) {
  return previous_length;
}

position_t singleplayer_state_get_previous_snake_segment(uint16_t index) {
  position_t empty = {0, 0};

  if (index >= previous_length)
    return empty;

  return previous_body[index];
}

uint8_t singleplayer_state_get_animation_step(void) {
  return movement_tick_counter;
}

uint8_t singleplayer_state_get_animation_steps(void) {
  return SINGLEPLAYER_MOVE_TICKS > 1 ? SINGLEPLAYER_MOVE_TICKS - 1 : 1;
}

bool singleplayer_state_is_death_animating(void) {
  return death_animating;
}

uint16_t singleplayer_state_get_death_animation_tick(void) {
  return death_animation_tick;
}

uint16_t singleplayer_state_get_death_animation_ticks(void) {
  return SINGLEPLAYER_DEATH_ANIMATION_TICKS;
}

uint8_t singleplayer_state_get_effect_count(void) {
  uint8_t count = 0;

  for (uint8_t i = 0; i < SINGLEPLAYER_EFFECT_SLOTS; i++) {
    if (effects[i].active)
      count++;
  }

  return count;
}

powerup_type_t singleplayer_state_get_effect_type(uint8_t index) {
  uint8_t active_index = 0;

  for (uint8_t i = 0; i < SINGLEPLAYER_EFFECT_SLOTS; i++) {
    if (!effects[i].active)
      continue;

    if (active_index == index)
      return effects[i].type;

    active_index++;
  }

  return POWERUP_DOUBLE_POINTS;
}

uint16_t singleplayer_state_get_effect_ticks(uint8_t index) {
  uint8_t active_index = 0;

  for (uint8_t i = 0; i < SINGLEPLAYER_EFFECT_SLOTS; i++) {
    if (!effects[i].active)
      continue;

    if (active_index == index)
      return effects[i].ticks;

    active_index++;
  }

  return 0;
}

bool singleplayer_state_has_shield(void) {
  return shield_charged;
}

bool singleplayer_state_is_bonus_food_active(void) {
  return singleplayer_effect_active(POWERUP_BONUS_FOOD);
}

bool singleplayer_state_has_held_powerup(void) {
  return has_held_powerup;
}

powerup_type_t singleplayer_state_get_held_powerup(void) {
  return held_powerup;
}

bool singleplayer_state_has_queued_powerup(void) {
  return has_queued_powerup;
}

powerup_type_t singleplayer_state_get_queued_powerup(void) {
  return queued_powerup;
}

const food_field_t *singleplayer_state_get_frenzy_field(void) {
  if (!ready)
    return NULL;

  return &frenzy_field;
}

void singleplayer_state_handle_timer(void) {
  singleplayer_state_ensure_ready();

  if (death_animating) {
    if (death_animation_tick < SINGLEPLAYER_DEATH_ANIMATION_TICKS)
      death_animation_tick++;

    if (death_animation_tick >= SINGLEPLAYER_DEATH_ANIMATION_TICKS)
      singleplayer_state_finish_game();

    return;
  }

  if (!snake_is_alive(&player_snake))
    return;

  singleplayer_effects_tick();
  singleplayer_update_frenzy_field();

  if (!powerup_is_active(&player_powerup)) {
    powerup_spawn_timer++;

    if (powerup_spawn_timer >= SINGLEPLAYER_POWERUP_SPAWN_TICKS) {
      powerup_spawn_from_types(&player_powerup,
                               &player_snake,
                               NULL,
                               &player_food,
                               singleplayer_powerup_types,
                               (uint8_t) (sizeof(singleplayer_powerup_types) / sizeof(singleplayer_powerup_types[0])));
      powerup_spawn_timer = 0;
    }
  }

  movement_tick_counter++;

  if (movement_tick_counter < SINGLEPLAYER_MOVE_TICKS)
    return;

  movement_tick_counter = 0;
  singleplayer_state_store_previous_snake();
  singleplayer_state_store_previous_food();

  bool hits_wall = collision_will_hit_wall(&player_snake);
  bool hits_self = collision_will_hit_self(&player_snake);
  bool ghost = singleplayer_effect_active(POWERUP_GHOST);
  bool fatal = hits_wall || (hits_self && !ghost);
  bool moved = false;

  if (fatal) {
    if (shield_charged) {
      shield_charged = false;

      if (singleplayer_state_shield_escape())
        moved = true;
      else {
        singleplayer_state_start_death_animation();
        return;
      }
    }
    else {
      singleplayer_state_start_death_animation();
      return;
    }
  }

  if (!moved) {
    if (hits_self && ghost)
      snake_move_ignoring_self(&player_snake);
    else
      snake_move(&player_snake);
  }

  if (!snake_is_alive(&player_snake)) {
    singleplayer_state_start_death_animation();
    return;
  }

  if (singleplayer_effect_active(POWERUP_MAGNET))
    singleplayer_state_apply_magnet();

  if (food_collides_with_snake_head(&player_food, &player_snake)) {
    snake_grow(&player_snake);

    if (singleplayer_effect_active(POWERUP_BONUS_FOOD))
      score_add_points(&player_score, SINGLEPLAYER_BONUS_FOOD_POINTS);
    else
      score_add_food(&player_score);

    food_spawn(&player_food, &player_snake);
    singleplayer_state_store_previous_food();
  }

  if (singleplayer_effect_active(POWERUP_FRENZY)) {
    int16_t hit = food_field_collides_with_snake_head(&frenzy_field, &player_snake);

    if (hit >= 0) {
      score_add_food(&player_score);
      food_field_respawn(&frenzy_field, (uint8_t) hit, &player_snake, &player_food);
    }
  }

  if (powerup_collides_with_snake_head(&player_powerup, &player_snake)) {
    powerup_type_t type = powerup_get_type(&player_powerup);

    if (type == POWERUP_BONUS_FOOD)
      singleplayer_state_activate_powerup(type);
    else {
      if (!has_held_powerup) {
        held_powerup = type;
        has_held_powerup = true;
      }
      else {
        queued_powerup = type;
        has_queued_powerup = true;
      }
    }

    powerup_clear(&player_powerup);
    powerup_spawn_timer = 0;
  }
}

void singleplayer_state_handle_keyboard(uint8_t scancode) {
  singleplayer_state_ensure_ready();

  if (death_animating)
    return;

  switch (scancode) {
    case STATE_KEY_ESC_BREAK:
    case STATE_KEY_P_MAKE:
      state_set_current(STATE_PAUSE);
      break;

    case STATE_KEY_SPACE_MAKE:
      singleplayer_state_activate_held_powerup();
      break;

    case STATE_KEY_W_MAKE:
    case STATE_KEY_UP_MAKE:
      snake_set_next_direction(&player_snake, DIRECTION_UP);
      break;

    case STATE_KEY_D_MAKE:
    case STATE_KEY_RIGHT_MAKE:
      snake_set_next_direction(&player_snake, DIRECTION_RIGHT);
      break;

    case STATE_KEY_S_MAKE:
    case STATE_KEY_DOWN_MAKE:
      snake_set_next_direction(&player_snake, DIRECTION_DOWN);
      break;

    case STATE_KEY_A_MAKE:
    case STATE_KEY_LEFT_MAKE:
      snake_set_next_direction(&player_snake, DIRECTION_LEFT);
      break;

    default:
      break;
  }
}

void singleplayer_state_handle_mouse(const struct packet *packet) {
  (void) packet;

  singleplayer_state_ensure_ready();

  if (death_animating)
    return;

  if (mouse_cursor_was_left_clicked())
    singleplayer_state_activate_held_powerup();
}
