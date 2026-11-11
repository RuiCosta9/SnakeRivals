#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file ui_layout.h
 * @brief Shared menu rectangle definitions and hit-testing helpers.
 */

/**
 * @brief Axis-aligned rectangle in screen coordinates.
 */
typedef struct {
  uint16_t x;      /**< Left coordinate. */
  uint16_t y;      /**< Top coordinate. */
  uint16_t width;  /**< Width in pixels. */
  uint16_t height; /**< Height in pixels. */
} ui_rect_t;

/**
 * @brief Main menu button identifiers.
 */
typedef enum {
  MENU_BUTTON_PLAY,         /**< Open play menu. */
  MENU_BUTTON_HOST,         /**< Host multiplayer directly. */
  MENU_BUTTON_INSTRUCTIONS, /**< Open instructions. */
  MENU_BUTTON_EXIT,         /**< Exit the application. */
  MENU_BUTTON_COUNT         /**< Number of main menu buttons. */
} menu_button_t;

/**
 * @brief Play menu button identifiers.
 */
typedef enum {
  PLAY_MENU_BUTTON_SINGLEPLAYER,      /**< Start local game. */
  PLAY_MENU_BUTTON_JOIN_MULTIPLAYER,  /**< Open room-code join screen. */
  PLAY_MENU_BUTTON_BACK,              /**< Return to main menu. */
  PLAY_MENU_BUTTON_COUNT              /**< Number of play menu buttons. */
} play_menu_button_t;


/**
 * @brief Join screen button identifiers.
 */
typedef enum {
  JOIN_BUTTON_JOIN, /**< Submit room code. */
  JOIN_BUTTON_BACK, /**< Return to menu. */
  JOIN_BUTTON_COUNT /**< Number of join buttons. */
} join_button_t;

/**
 * @brief Pause menu button identifiers.
 */
typedef enum {
  PAUSE_BUTTON_RESUME,    /**< Resume the current game. */
  PAUSE_BUTTON_RESTART,   /**< Restart singleplayer. */
  PAUSE_BUTTON_MAIN_MENU, /**< Return to main menu. */
  PAUSE_BUTTON_EXIT,      /**< Exit the application. */
  PAUSE_BUTTON_COUNT      /**< Number of pause buttons. */
} pause_button_t;

/**
 * @brief Game-over menu button identifiers.
 */
typedef enum {
  GAME_OVER_BUTTON_RESTART,   /**< Start a new game. */
  GAME_OVER_BUTTON_MAIN_MENU, /**< Return to main menu. */
  GAME_OVER_BUTTON_EXIT,      /**< Exit the application. */
  GAME_OVER_BUTTON_COUNT      /**< Number of game-over buttons. */
} game_over_button_t;

/**
 * @brief Multiplayer lobby button identifiers.
 */
typedef enum {
  MULTIPLAYER_LOBBY_BUTTON_SCORE_RACE,   /**< Select Score Race mode. */
  MULTIPLAYER_LOBBY_BUTTON_ARENA_BATTLE, /**< Select Arena Battle mode. */
  MULTIPLAYER_LOBBY_BUTTON_PLAYER1_COLOR,/**< Change player 1 skin. */
  MULTIPLAYER_LOBBY_BUTTON_PLAYER2_COLOR,/**< Change player 2 skin. */
  MULTIPLAYER_LOBBY_BUTTON_START,        /**< Start match as host. */
  MULTIPLAYER_LOBBY_BUTTON_BACK,         /**< Leave lobby. */
  MULTIPLAYER_LOBBY_BUTTON_COUNT         /**< Number of lobby buttons. */
} multiplayer_lobby_button_t;

/**
 * @brief Check whether a point lies inside a rectangle.
 *
 * @param rect Rectangle to inspect.
 * @param x Point x coordinate.
 * @param y Point y coordinate.
 * @return true if the point is inside.
 */
bool ui_rect_contains(const ui_rect_t *rect, uint16_t x, uint16_t y);

/** @brief Get a main menu button rectangle. @param button Button id. @return Rectangle. */
ui_rect_t ui_layout_get_menu_button(menu_button_t button);

/** @brief Get a play menu button rectangle. @param button Button id. @return Rectangle. */
ui_rect_t ui_layout_get_play_menu_button(play_menu_button_t button);

/** @brief Get a pause menu button rectangle. @param button Button id. @return Rectangle. */
ui_rect_t ui_layout_get_pause_button(pause_button_t button);

/** @brief Get the instructions back button rectangle. @return Rectangle. */
ui_rect_t ui_layout_get_instructions_back_button(void);

/** @brief Get a join screen button rectangle. @param button Button id. @return Rectangle. */
ui_rect_t ui_layout_get_join_button(join_button_t button);

/** @brief Get a game-over button rectangle. @param button Button id. @return Rectangle. */
ui_rect_t ui_layout_get_game_over_button(game_over_button_t button);

/** @brief Get a multiplayer lobby button rectangle. @param button Button id. @return Rectangle. */
ui_rect_t ui_layout_get_multiplayer_lobby_button(multiplayer_lobby_button_t button);

/** @brief Get the multiplayer game-over return button rectangle. @return Rectangle. */
ui_rect_t ui_layout_get_multiplayer_game_over_button(void);

#endif
