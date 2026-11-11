/**
 * @file multiplayer_effects.c
 * @brief Host-side timed effect slot management for multiplayer.
 */

#include "multiplayer_effects.h"

#include <stdbool.h>
#include <stdint.h>

#include "multiplayer_session.h"
#include "multiplayer_session_internal.h"

static uint16_t multiplayer_effects_duration(powerup_type_t type) {
  switch (type) {
    case POWERUP_DOUBLE_POINTS:
      return MULTIPLAYER_SPEED_BOOST_TICKS;

    case POWERUP_SPEED_BOOST:
      return MULTIPLAYER_SPEED_BOOST_TICKS;

    case POWERUP_SLOW_ENEMY:
      return MULTIPLAYER_SLOW_ENEMY_TICKS;

    case POWERUP_GHOST:
      return MULTIPLAYER_GHOST_TICKS;

    case POWERUP_MAGNET:
      return MULTIPLAYER_MAGNET_TICKS;

    case POWERUP_FRENZY:
      return MULTIPLAYER_FRENZY_TICKS;

    default:
      return 0;
  }
}

bool multiplayer_effects_active(uint8_t player_index, powerup_type_t type) {
  if (player_index >= 2)
    return false;

  for (uint8_t i = 0; i < MULTIPLAYER_EFFECT_SLOTS; i++) {
    if (host_effects[player_index][i].active && host_effects[player_index][i].type == type)
      return true;
  }

  return false;
}

void multiplayer_effects_add(uint8_t player_index, powerup_type_t type) {
  if (player_index >= 2)
    return;

  uint16_t duration = multiplayer_effects_duration(type);

  if (duration == 0)
    return;

  for (uint8_t i = 0; i < MULTIPLAYER_EFFECT_SLOTS; i++) {
    if (host_effects[player_index][i].active && host_effects[player_index][i].type == type) {
      host_effects[player_index][i].ticks = duration;
      return;
    }
  }

  for (uint8_t i = 0; i < MULTIPLAYER_EFFECT_SLOTS; i++) {
    if (!host_effects[player_index][i].active) {
      host_effects[player_index][i].type = type;
      host_effects[player_index][i].ticks = duration;
      host_effects[player_index][i].active = true;
      return;
    }
  }

  for (uint8_t i = 1; i < MULTIPLAYER_EFFECT_SLOTS; i++)
    host_effects[player_index][i - 1] = host_effects[player_index][i];

  host_effects[player_index][MULTIPLAYER_EFFECT_SLOTS - 1].type = type;
  host_effects[player_index][MULTIPLAYER_EFFECT_SLOTS - 1].ticks = duration;
  host_effects[player_index][MULTIPLAYER_EFFECT_SLOTS - 1].active = true;
}

void multiplayer_effects_tick_all(void) {
  for (uint8_t player = 0; player < 2; player++) {
    for (uint8_t slot = 0; slot < MULTIPLAYER_EFFECT_SLOTS; slot++) {
      if (host_effects[player][slot].active && host_effects[player][slot].ticks > 0) {
        host_effects[player][slot].ticks--;

        if (host_effects[player][slot].ticks == 0)
          host_effects[player][slot].active = false;
      }
    }

    for (uint8_t slot = 1; slot < MULTIPLAYER_EFFECT_SLOTS; slot++) {
      if (!host_effects[player][slot - 1].active && host_effects[player][slot].active) {
        host_effects[player][slot - 1] = host_effects[player][slot];
        host_effects[player][slot].active = false;
      }
    }
  }
}

void multiplayer_effects_clear_all(void) {
  for (uint8_t player = 0; player < 2; player++) {
    for (uint8_t slot = 0; slot < MULTIPLAYER_EFFECT_SLOTS; slot++) {
      host_effects[player][slot].type = POWERUP_DOUBLE_POINTS;
      host_effects[player][slot].ticks = 0;
      host_effects[player][slot].active = false;
    }
  }
}
