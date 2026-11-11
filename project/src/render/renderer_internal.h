#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include "food.h"
#include "multiplayer_session.h"
#include "powerup.h"
#include "renderer.h"
#include "snake.h"
#include "sprite.h"
#include "ui_layout.h"

/**
 * @file renderer_internal.h
 * @brief Private renderer subsystem helpers shared by renderer_*.c files.
 *
 * These declarations keep each renderer file focused on one screen or mode
 * while sharing drawing primitives for snakes, buttons, inventory and arenas.
 */

/* Shared renderer helpers implemented inside the render subsystem. */
int renderer_submit_cursor(void);
int renderer_submit_centered_text(const char *text, uint16_t center_x, uint16_t y, uint8_t scale, uint32_t color, int16_t z);
int renderer_submit_arena_border(uint16_t arena_x, uint16_t arena_y, uint16_t arena_w, uint16_t arena_h, uint16_t border, uint32_t color);
int renderer_submit_play_background(uint16_t area_x, uint16_t area_y, uint16_t area_w, uint16_t area_h, int32_t scroll_x, int32_t scroll_y);
int renderer_submit_snake_cells(const snake_t *snake, uint16_t arena_x, uint16_t arena_y, uint16_t cell_size, uint32_t body_color, uint32_t head_color, int16_t z);
int renderer_submit_powerup_icon(powerup_type_t type, uint16_t x, uint16_t y, uint16_t cell_size, int16_t z);
int renderer_submit_powerup_inventory_slot(uint16_t x, uint16_t y, uint16_t size, const char *label, bool filled, powerup_type_t type, int16_t z);
uint32_t renderer_powerup_color(powerup_type_t type);
const char *renderer_powerup_label(powerup_type_t type);
direction_t renderer_get_snake_head_direction(const snake_t *snake);
direction_t renderer_get_snake_tail_direction(const snake_t *snake);

/* State rendering functions. */
int renderer_draw_menu(void);
int renderer_draw_play_menu(void);
int renderer_draw_join_menu(void);
int renderer_draw_instructions(void);
int renderer_draw_singleplayer(void);
int renderer_draw_pause(void);
int renderer_draw_game_over(void);

int renderer_submit_button_visual(const ui_rect_t *rect, const char *label, int16_t z);
int renderer_submit_button_visual_state(const ui_rect_t *rect, const char *label, bool selected, int16_t z);
int renderer_submit_button_icon(const sprite_t *sprite, const ui_rect_t *rect, int16_t z);
int renderer_submit_back_arrow(uint16_t x, uint16_t y, uint16_t size, int16_t z);
int renderer_submit_snake(const snake_t *snake, uint16_t arena_x, uint16_t arena_y, uint16_t cell_size, int16_t body_layer, int16_t head_layer);
int renderer_submit_death_snake_grid(const snake_t *snake, uint16_t arena_x, uint16_t arena_y, uint16_t cell_size, uint16_t tick, uint16_t total_ticks, int16_t body_layer, int16_t head_layer);
int renderer_submit_death_snake_grid_skin(const snake_t *snake, uint8_t skin, uint16_t arena_x, uint16_t arena_y, uint16_t cell_size, uint16_t tick, uint16_t total_ticks, int16_t body_layer, int16_t head_layer);
int renderer_submit_death_snake_camera(const snake_t *snake, int32_t camera_x, int32_t camera_y, uint16_t cell_size, uint16_t y_offset, uint16_t viewport_h, uint16_t tick, uint16_t total_ticks, int16_t body_layer, int16_t head_layer);
int renderer_submit_death_snake_camera_skin(const snake_t *snake, uint8_t skin, int32_t camera_x, int32_t camera_y, uint16_t cell_size, uint16_t y_offset, uint16_t viewport_h, uint16_t tick, uint16_t total_ticks, int16_t body_layer, int16_t head_layer);

/* Multiplayer lobby rendering. */
int renderer_draw_host_lobby(uint16_t hres, uint16_t vres);
int renderer_draw_client_lobby(uint16_t hres, uint16_t vres);

/* Multiplayer result/dispatcher rendering. */
int renderer_draw_multiplayer_result_overlay(uint16_t hres, uint16_t vres);
int renderer_draw_multiplayer(void);

/* Score Race rendering. */
int renderer_submit_snake_minimap(const snake_t *snake, uint16_t panel_x, uint16_t panel_y, uint16_t cell_size, uint32_t color, int16_t z);
int renderer_submit_score_mode_minimap(const snake_t *opponent, uint16_t hres);
int renderer_submit_multiplayer_player_food(uint8_t player_id, uint16_t arena_x, uint16_t arena_y);
int renderer_submit_multiplayer_player_powerup(uint8_t player_id, uint16_t arena_x, uint16_t arena_y);
int renderer_submit_multiplayer_inventory(uint8_t player_id, uint16_t x, uint16_t y);
int renderer_submit_multiplayer_scores(uint16_t arena_x, uint16_t arena_y, uint16_t arena_w);
int renderer_draw_multiplayer_score(uint16_t hres, uint16_t vres, multiplayer_role_t role, const snake_t *player1, const snake_t *player2);

/* Arena Battle rendering. */
int32_t renderer_clamp_i32(int32_t value, int32_t min_value, int32_t max_value);
int renderer_submit_snake_camera(const snake_t *snake, int32_t camera_x, int32_t camera_y, uint16_t cell_size, int16_t body_layer, int16_t head_layer);
int renderer_submit_item_camera(const food_t *item, int32_t camera_x, int32_t camera_y, uint16_t cell_size, uint32_t color, int16_t z);
int renderer_submit_world_border_camera(int32_t camera_x, int32_t camera_y, uint16_t world_w, uint16_t world_h, uint16_t cell_size);
int renderer_submit_arena_walls_camera(int32_t camera_x, int32_t camera_y, uint16_t cell_size, uint16_t y_offset, uint16_t viewport_h);
int renderer_draw_multiplayer_arena(uint16_t hres, uint16_t vres, multiplayer_role_t role, const snake_t *player1, const snake_t *player2);

#endif
