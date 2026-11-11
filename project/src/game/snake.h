#ifndef SNAKE_H
#define SNAKE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file snake.h
 * @brief Grid-based snake model and movement rules.
 *
 * The snake module stores body segments, direction state and alive/dead state.
 * It does not render anything and can be reused by singleplayer, multiplayer
 * and protocol snapshots.
 */

#define SNAKE_MAX_SEGMENTS 256
#define SNAKE_INITIAL_LENGTH 4

/**
 * @brief Integer position inside a grid.
 */
typedef struct {
  int16_t x; /**< Column index. */
  int16_t y; /**< Row index. */
} position_t;

/**
 * @brief Cardinal movement direction.
 */
typedef enum {
  DIRECTION_UP,    /**< Move one cell up. */
  DIRECTION_RIGHT, /**< Move one cell right. */
  DIRECTION_DOWN,  /**< Move one cell down. */
  DIRECTION_LEFT   /**< Move one cell left. */
} direction_t;

/**
 * @brief Complete snake state used by the game logic.
 */
typedef struct {
  position_t body[SNAKE_MAX_SEGMENTS]; /**< Body segments, with head at index 0. */
  uint16_t length;                      /**< Number of valid body segments. */
  uint16_t grid_width;                  /**< Grid width in cells. */
  uint16_t grid_height;                 /**< Grid height in cells. */
  direction_t current_direction;        /**< Direction used by the latest move. */
  direction_t next_direction;           /**< Direction requested for the next move. */
  bool alive;                           /**< Whether the snake can still move. */
} snake_t;

/**
 * @brief Initialize a snake at the center of a grid.
 *
 * @param snake Snake to initialize.
 * @param grid_width Grid width in cells.
 * @param grid_height Grid height in cells.
 */
void snake_init(snake_t *snake, uint16_t grid_width, uint16_t grid_height);

/**
 * @brief Replace the snake body with explicit segments.
 *
 * @param snake Snake to update.
 * @param segments Segment array with head at index 0.
 * @param length Number of segments to copy.
 * @param direction Direction assigned to both current and next direction.
 * @return 0 on success, non-zero on invalid input.
 *
 * @note Used by the multiplayer protocol when rebuilding snapshot snakes.
 */
int snake_set_body(snake_t *snake, const position_t *segments, uint16_t length, direction_t direction);

/**
 * @brief Queue a direction for the next movement step.
 *
 * @param snake Snake to update.
 * @param direction Requested direction.
 *
 * @note Opposite turns are ignored to avoid instant self-collision.
 */
void snake_set_next_direction(snake_t *snake, direction_t direction);

/**
 * @brief Force the alive/dead state of a snake.
 *
 * @param snake Snake to update.
 * @param alive New alive flag.
 */
void snake_set_alive(snake_t *snake, bool alive);

/**
 * @brief Move the snake one cell using its queued direction.
 *
 * @param snake Snake to move.
 *
 * @note Sets alive to false on wall or self collision.
 */
void snake_move(snake_t *snake);

/**
 * @brief Move the snake one cell while ignoring body self-collision.
 *
 * @param snake Snake to move.
 *
 * @note Wall collisions are still fatal.
 */
void snake_move_ignoring_self(snake_t *snake);

/**
 * @brief Check whether a direction would immediately kill the snake.
 *
 * @param snake Snake to inspect.
 * @param direction Direction to test.
 * @return true if the move would hit a wall/body or input is invalid.
 */
bool snake_direction_would_be_fatal(const snake_t *snake, direction_t direction);

/**
 * @brief Grow the snake by duplicating its tail segment.
 *
 * @param snake Snake to grow.
 */
void snake_grow(snake_t *snake);

/**
 * @brief Get the number of segments in a snake.
 *
 * @param snake Snake to inspect.
 * @return Segment count, or 0 for NULL.
 */
uint16_t snake_get_length(const snake_t *snake);

/**
 * @brief Get a body segment.
 *
 * @param snake Snake to inspect.
 * @param index Segment index.
 * @return Segment position, or {0,0} for invalid input.
 */
position_t snake_get_segment(const snake_t *snake, uint16_t index);

/**
 * @brief Get the head position.
 *
 * @param snake Snake to inspect.
 * @return Head position, or {0,0} for invalid input.
 */
position_t snake_get_head(const snake_t *snake);

/**
 * @brief Get the direction used by the latest movement.
 *
 * @param snake Snake to inspect.
 * @return Current direction, or DIRECTION_RIGHT for NULL.
 */
direction_t snake_get_current_direction(const snake_t *snake);

/**
 * @brief Get the direction queued for the next movement.
 *
 * @param snake Snake to inspect.
 * @return Next direction, or DIRECTION_RIGHT for NULL.
 */
direction_t snake_get_next_direction(const snake_t *snake);

/**
 * @brief Check whether the snake is alive.
 *
 * @param snake Snake to inspect.
 * @return true if alive, false for dead or NULL.
 */
bool snake_is_alive(const snake_t *snake);

#endif
