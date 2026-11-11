/**
 * @file menu_nav.c
 * @brief Generic menu keyboard/mouse navigation implementation.
 */

#include "menu_nav.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "state.h"

bool menu_nav_handle_keyboard(uint8_t scancode, uint8_t *selected, uint8_t count, bool *activate) {
  if (activate != NULL)
    *activate = false;

  if (selected == NULL || count == 0)
    return false;

  switch (scancode) {
    case STATE_KEY_W_MAKE:
    case STATE_KEY_UP_MAKE:
      *selected = (uint8_t) ((*selected + count - 1) % count);
      return true;

    case STATE_KEY_S_MAKE:
    case STATE_KEY_DOWN_MAKE:
      *selected = (uint8_t) ((*selected + 1) % count);
      return true;

    case STATE_KEY_ENTER_MAKE:
      if (activate != NULL)
        *activate = true;
      return true;

    default:
      return false;
  }
}

void menu_nav_update_hover(menu_nav_rect_provider_t provider, uint8_t count, uint16_t x, uint16_t y, uint8_t *selected) {
  if (provider == NULL || selected == NULL)
    return;

  for (uint8_t i = 0; i < count; i++) {
    ui_rect_t rect = provider(i);

    if (ui_rect_contains(&rect, x, y)) {
      *selected = i;
      return;
    }
  }
}

bool menu_nav_handle_mouse(menu_nav_rect_provider_t provider, uint8_t count, uint16_t x, uint16_t y, bool clicked, uint8_t *selected, uint8_t *activated) {
  if (activated != NULL)
    *activated = 0;

  if (provider == NULL || selected == NULL)
    return false;

  for (uint8_t i = 0; i < count; i++) {
    ui_rect_t rect = provider(i);

    if (!ui_rect_contains(&rect, x, y))
      continue;

    *selected = i;

    if (clicked) {
      if (activated != NULL)
        *activated = i;
      return true;
    }

    return false;
  }

  return false;
}
