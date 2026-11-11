#ifndef APP_H
#define APP_H

/**
 * @file app.h
 * @brief Application lifecycle entry points.
 *
 * Owns startup, the interrupt-driven main loop and shutdown of the game.
 */

/**
 * @brief Initialize graphics, assets, renderer and device subscriptions.
 *
 * @return 0 on success, non-zero on failure.
 *
 * @note On partial failure the implementation attempts to clean up resources
 * already acquired.
 */
int app_init(void);

/**
 * @brief Run the event loop until the global state becomes STATE_EXIT.
 *
 * @return 0 when the loop exits normally.
 */
int app_run(void);

/**
 * @brief Release device subscriptions, graphics resources and loaded assets.
 *
 * @return 0 on success, non-zero if any cleanup step fails.
 *
 * @note Forces the state machine into STATE_EXIT.
 */
int app_cleanup(void);

#endif
