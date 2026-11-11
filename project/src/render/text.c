/**
 * @file text.c
 * @brief Built-in bitmap font drawing implementation.
 */

#include "text.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "video.h"

static const char font_space[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "...",
  "...",
  "...",
  "...",
  "..."
};

static const char font_unknown[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "..1",
  ".1.",
  "...",
  ".1."
};

static const char font_a[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  ".1.",
  "1.1",
  "111",
  "1.1",
  "1.1"
};

static const char font_b[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "11.",
  "1.1",
  "11.",
  "1.1",
  "11."
};

static const char font_c[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  ".11",
  "1..",
  "1..",
  "1..",
  ".11"
};

static const char font_d[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "11.",
  "1.1",
  "1.1",
  "1.1",
  "11."
};

static const char font_e[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "1..",
  "11.",
  "1..",
  "111"
};

static const char font_f[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "1..",
  "11.",
  "1..",
  "1.."
};

static const char font_g[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  ".11",
  "1..",
  "1.1",
  "1.1",
  ".11"
};

static const char font_h[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "1.1",
  "1.1",
  "111",
  "1.1",
  "1.1"
};

static const char font_i[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  ".1.",
  ".1.",
  ".1.",
  "111"
};

static const char font_j[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "..1",
  "..1",
  "..1",
  "1.1",
  ".1."
};

static const char font_k[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "1.1",
  "1.1",
  "11.",
  "1.1",
  "1.1"
};

static const char font_l[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "1..",
  "1..",
  "1..",
  "1..",
  "111"
};

static const char font_m[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "1.1",
  "111",
  "111",
  "1.1",
  "1.1"
};

static const char font_n[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "1.1",
  "111",
  "111",
  "111",
  "1.1"
};

static const char font_n_wide[TEXT_FONT_HEIGHT][5] = {
  "1..1",
  "11.1",
  "1.11",
  "1..1",
  "1..1"
};

static const char font_o[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "1.1",
  "1.1",
  "1.1",
  "111"
};

static const char font_p[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "11.",
  "1.1",
  "11.",
  "1..",
  "1.."
};

static const char font_q[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "1.1",
  "1.1",
  "111",
  "..1"
};

static const char font_r[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "11.",
  "1.1",
  "11.",
  "1.1",
  "1.1"
};

static const char font_s[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  ".11",
  "1..",
  ".1.",
  "..1",
  "11."
};

static const char font_t[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  ".1.",
  ".1.",
  ".1.",
  ".1."
};

static const char font_u[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "1.1",
  "1.1",
  "1.1",
  "1.1",
  "111"
};

static const char font_v[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "1.1",
  "1.1",
  "1.1",
  "1.1",
  ".1."
};

static const char font_w[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "1.1",
  "1.1",
  "111",
  "111",
  "1.1"
};

static const char font_x[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "1.1",
  "1.1",
  ".1.",
  "1.1",
  "1.1"
};

static const char font_y[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "1.1",
  "1.1",
  ".1.",
  ".1.",
  ".1."
};

static const char font_z[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "..1",
  ".1.",
  "1..",
  "111"
};

static const char font_0[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "1.1",
  "1.1",
  "1.1",
  "111"
};

static const char font_1[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  ".1.",
  "11.",
  ".1.",
  ".1.",
  "111"
};

static const char font_2[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "..1",
  "111",
  "1..",
  "111"
};

static const char font_3[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "..1",
  "111",
  "..1",
  "111"
};

static const char font_4[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "1.1",
  "1.1",
  "111",
  "..1",
  "..1"
};

static const char font_5[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "1..",
  "111",
  "..1",
  "111"
};

static const char font_6[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "1..",
  "111",
  "1.1",
  "111"
};

static const char font_7[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "..1",
  ".1.",
  ".1.",
  ".1."
};

static const char font_8[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "1.1",
  "111",
  "1.1",
  "111"
};

static const char font_9[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "111",
  "1.1",
  "111",
  "..1",
  "111"
};

static const char font_colon[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "...",
  ".1.",
  "...",
  ".1.",
  "..."
};

static const char font_dot[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "...",
  "...",
  "...",
  "...",
  ".1."
};

static const char font_dash[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "...",
  "...",
  "111",
  "...",
  "..."
};

static const char font_plus[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  ".1.",
  ".1.",
  "111",
  ".1.",
  ".1."
};

static const char font_slash[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "..1",
  "..1",
  ".1.",
  "1..",
  "1.."
};

static const char font_underscore[TEXT_FONT_HEIGHT][TEXT_FONT_WIDTH + 1] = {
  "...",
  "...",
  "...",
  "...",
  "111"
};

static char text_normalize_char(char c) {
  if (c >= 'a' && c <= 'z')
    return (char) (c - 'a' + 'A');

  return c;
}

static uint8_t text_get_char_width(char c) {
  c = text_normalize_char(c);

  if (c == 'N')
    return 4;

  return TEXT_FONT_WIDTH;
}

static const char (*text_get_glyph(char c))[TEXT_FONT_WIDTH + 1] {
  c = text_normalize_char(c);

  switch (c) {
    case 'A': return font_a;
    case 'B': return font_b;
    case 'C': return font_c;
    case 'D': return font_d;
    case 'E': return font_e;
    case 'F': return font_f;
    case 'G': return font_g;
    case 'H': return font_h;
    case 'I': return font_i;
    case 'J': return font_j;
    case 'K': return font_k;
    case 'L': return font_l;
    case 'M': return font_m;
    case 'N': return font_n;
    case 'O': return font_o;
    case 'P': return font_p;
    case 'Q': return font_q;
    case 'R': return font_r;
    case 'S': return font_s;
    case 'T': return font_t;
    case 'U': return font_u;
    case 'V': return font_v;
    case 'W': return font_w;
    case 'X': return font_x;
    case 'Y': return font_y;
    case 'Z': return font_z;
    case '0': return font_0;
    case '1': return font_1;
    case '2': return font_2;
    case '3': return font_3;
    case '4': return font_4;
    case '5': return font_5;
    case '6': return font_6;
    case '7': return font_7;
    case '8': return font_8;
    case '9': return font_9;
    case ':': return font_colon;
    case '.': return font_dot;
    case '-': return font_dash;
    case '+': return font_plus;
    case '/': return font_slash;
    case '_': return font_underscore;
    case ' ': return font_space;
    default: return font_unknown;
  }
}

uint16_t text_width(const char *text, uint8_t scale) {
  if (text == NULL || scale == 0)
    return 0;

  uint16_t width = 0;

  for (size_t i = 0; text[i] != '\0'; i++)
    width += (text_get_char_width(text[i]) + TEXT_FONT_SPACING) * scale;

  return width;
}

int text_draw(const char *text,
              uint16_t x,
              uint16_t y,
              uint8_t scale,
              uint32_t color) {
  if (text == NULL || scale == 0)
    return 1;

  uint16_t cursor_x = x;

  for (size_t i = 0; text[i] != '\0'; i++) {
    char c = text_normalize_char(text[i]);
    uint8_t glyph_width = text_get_char_width(c);

    if (c == 'N') {
      for (uint8_t row = 0; row < TEXT_FONT_HEIGHT; row++) {
        for (uint8_t col = 0; col < glyph_width; col++) {
          if (font_n_wide[row][col] == '1') {
            if (video_draw_rectangle(cursor_x + col * scale, y + row * scale, scale, scale, color) != 0)
              return 1;
          }
        }
      }
    }
    else {
      const char (*glyph)[TEXT_FONT_WIDTH + 1] = text_get_glyph(c);

      for (uint8_t row = 0; row < TEXT_FONT_HEIGHT; row++) {
        for (uint8_t col = 0; col < glyph_width; col++) {
          if (glyph[row][col] == '1') {
            if (video_draw_rectangle(cursor_x + col * scale, y + row * scale, scale, scale, color) != 0)
              return 1;
          }
        }
      }
    }

    cursor_x += (glyph_width + TEXT_FONT_SPACING) * scale;
  }

  return 0;
}
