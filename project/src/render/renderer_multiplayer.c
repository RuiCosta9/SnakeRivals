/**
 * @file renderer_multiplayer.c
 * @brief Multiplayer render dispatcher and common match overlays.
 */

#include "renderer_internal.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "assets.h"
#include "food.h"
#include "multiplayer_session.h"
#include "snake.h"
#include "text.h"
#include "ui_layout.h"
#include "vbe.h"
int renderer_draw_multiplayer_result_overlay(uint16_t hres, uint16_t vres) {
  multiplayer_match_phase_t phase = multiplayer_session_get_match_phase();

  if (phase == MP_MATCH_COUNTDOWN) {
    char countdown_text[8];
    uint8_t seconds = multiplayer_session_get_countdown_seconds();

    if (seconds == 0)
      snprintf(countdown_text, sizeof(countdown_text), "GO");
    else
      snprintf(countdown_text, sizeof(countdown_text), "%u", (unsigned int) seconds);

    if (renderer_submit_rect(hres / 2 - 80, vres / 2 - 80, 160, 160, 0x00, RENDER_LAYER_UI + 20) != 0)
      return 1;

    if (renderer_submit_text(countdown_text, hres / 2 - text_width(countdown_text, 8) / 2, vres / 2 - 40, 8, 0x0F, RENDER_LAYER_UI + 21) != 0)
      return 1;
  }

  if (phase == MP_MATCH_FINISHED) {
    char winner_text[32];
    char p1_score_text[24];
    char p2_score_text[24];
    multiplayer_winner_t result = multiplayer_session_get_winner();
    ui_rect_t button = ui_layout_get_multiplayer_game_over_button();

    if (result == MP_WINNER_PLAYER_1)
      snprintf(winner_text, sizeof(winner_text), "PLAYER 1 WINS");
    else if (result == MP_WINNER_PLAYER_2)
      snprintf(winner_text, sizeof(winner_text), "PLAYER 2 WINS");
    else
      snprintf(winner_text, sizeof(winner_text), "DRAW");

    snprintf(p1_score_text, sizeof(p1_score_text), "PLAYER 1 SCORE %u", (unsigned int) multiplayer_session_get_player_score(1));
    snprintf(p2_score_text, sizeof(p2_score_text), "PLAYER 2 SCORE %u", (unsigned int) multiplayer_session_get_player_score(2));

    if (renderer_submit_rect(hres / 2 - 230, vres / 2 - 126, 460, 292, 0x00, RENDER_LAYER_UI + 20) != 0)
      return 1;

    if (renderer_submit_rect(hres / 2 - 222, vres / 2 - 118, 444, 276, 0x01, RENDER_LAYER_UI + 21) != 0)
      return 1;

    if (renderer_submit_text(winner_text, hres / 2 - text_width(winner_text, 5) / 2, vres / 2 - 96, 5, 0x0F, RENDER_LAYER_UI + 22) != 0)
      return 1;

    if (renderer_submit_text(p1_score_text, hres / 2 - text_width(p1_score_text, 2) / 2, vres / 2 - 20, 2, 0x0A, RENDER_LAYER_UI + 22) != 0)
      return 1;

    if (renderer_submit_text(p2_score_text, hres / 2 - text_width(p2_score_text, 2) / 2, vres / 2 + 8, 2, 0x0B, RENDER_LAYER_UI + 22) != 0)
      return 1;

    const char *button_label = multiplayer_session_get_role() == MP_ROLE_HOST ? "BACK TO LOBBY" : "WAITING FOR LEADER";
    bool button_enabled = multiplayer_session_get_role() == MP_ROLE_HOST;

    if (renderer_submit_button_visual_state(&button, button_label, button_enabled, RENDER_LAYER_UI + 22) != 0)
      return 1;
  }

  return 0;
}

int renderer_draw_multiplayer(void) {
  uint16_t hres = vbe_get_hres();
  uint16_t vres = vbe_get_vres();

  multiplayer_role_t role = multiplayer_session_get_role();
  multiplayer_connection_state_t connection_state = multiplayer_session_get_connection_state();
  multiplayer_game_mode_t game_mode = multiplayer_session_get_game_mode();
  multiplayer_match_phase_t phase = multiplayer_session_get_match_phase();

  if (renderer_submit_rect(0, 0, hres, vres, 0x00, RENDER_LAYER_BACKGROUND) != 0)
    return 1;

  if (role == MP_ROLE_HOST && phase == MP_MATCH_LOBBY)
    return renderer_draw_host_lobby(hres, vres);

  if (role == MP_ROLE_CLIENT && multiplayer_session_join_was_rejected()) {
    if (renderer_submit_text("INVALID CODE", hres / 2 - text_width("INVALID CODE", 5) / 2, vres / 3, 5, 0x0F, RENDER_LAYER_UI) != 0)
      return 1;

    if (renderer_submit_text("ESC TO BACK", hres / 2 - text_width("ESC TO BACK", 3) / 2, vres / 2, 3, 0x0E, RENDER_LAYER_UI) != 0)
      return 1;

    return 0;
  }

  if (role == MP_ROLE_CLIENT && connection_state != MP_CONNECTION_CONNECTED) {
    if (renderer_submit_text("JOINING GAME", hres / 2 - text_width("JOINING GAME", 5) / 2, vres / 3, 5, 0x0F, RENDER_LAYER_UI) != 0)
      return 1;

    if (renderer_submit_text("WAITING FOR HOST", hres / 2 - text_width("WAITING FOR HOST", 3) / 2, vres / 2, 3, 0x0E, RENDER_LAYER_UI) != 0)
      return 1;

    if (renderer_submit_text("ESC TO BACK", hres / 2 - text_width("ESC TO BACK", 2) / 2, vres - 72, 2, 0x08, RENDER_LAYER_UI) != 0)
      return 1;

    return 0;
  }

  if (role == MP_ROLE_CLIENT && phase == MP_MATCH_LOBBY)
    return renderer_draw_client_lobby(hres, vres);

  const snake_t *player1 = multiplayer_session_get_player_snake(1);
  const snake_t *player2 = multiplayer_session_get_player_snake(2);

  if (game_mode == MP_GAME_MODE_SCORE)
    return renderer_draw_multiplayer_score(hres, vres, role, player1, player2);

  return renderer_draw_multiplayer_arena(hres, vres, role, player1, player2);
}
