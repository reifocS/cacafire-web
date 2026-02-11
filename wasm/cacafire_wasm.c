/*
 *  cacafire_wasm.c - Fire algorithm + WASM API wrapper
 *
 *  Fire algorithm extracted from libcaca/src/aafire.c
 *  Copyright (c) 2003-2010 Sam Hocevar <sam@hocevar.net>
 *                1998, 1999, 2001 Jan Hubicka <hubicka@freesoft.cz>
 *
 *  WASM wrapper by cacafire-web project.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "caca.h"

/* Use our deterministic PRNG instead of stdlib rand() */
#include "prng.h"
#define rand() prng_rand()

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT
#endif

/* --- Stubs for symbols not needed in headless mode --- */

int caca_canvas_set_figfont(caca_canvas_t *cv, char const *path)
{
    (void)cv;
    (void)path;
    return 0;
}

/* --- Fire constants --- */

#define MAXTABLE (256 * 5)

/* --- Static state --- */

static caca_canvas_t *cv;
static caca_dither_t *caca_dither;
static char *bitmap;
static unsigned int table[MAXTABLE];
static int XSIZ, YSIZ;

/* drawfire state (moved from static locals to file scope for fire_resize) */
static unsigned int height = 0;
static int loop = 0, sloop = 0;

/* --- Palette from aafire.c (lines 53-89) --- */

static int const pal[] =
{
  0, 0, 0, 0, 0, 6, 0, 0, 6, 0, 0, 7, 0, 0, 8, 0, 0, 8, 0, 0, 9, 0, 0, 10,
2, 0, 10, 4, 0, 9, 6, 0, 9, 8, 0, 8, 10, 0, 7, 12, 0, 7, 14, 0, 6, 16, 0, 5,
  18, 0, 5, 20, 0, 4, 22, 0, 4, 24, 0, 3, 26, 0, 2, 28, 0, 2, 30, 0, 1, 32, 0, 0,
  32, 0, 0, 33, 0, 0, 34, 0, 0, 35, 0, 0, 36, 0, 0, 36, 0, 0, 37, 0, 0, 38, 0, 0,
  39, 0, 0, 40, 0, 0, 40, 0, 0, 41, 0, 0, 42, 0, 0, 43, 0, 0, 44, 0, 0, 45, 0, 0,
  46, 1, 0, 47, 1, 0, 48, 2, 0, 49, 2, 0, 50, 3, 0, 51, 3, 0, 52, 4, 0, 53, 4, 0,
  54, 5, 0, 55, 5, 0, 56, 6, 0, 57, 6, 0, 58, 7, 0, 59, 7, 0, 60, 8, 0, 61, 8, 0,
  63, 9, 0, 63, 9, 0, 63, 10, 0, 63, 10, 0, 63, 11, 0, 63, 11, 0, 63, 12, 0, 63, 12, 0,
  63, 13, 0, 63, 13, 0, 63, 14, 0, 63, 14, 0, 63, 15, 0, 63, 15, 0, 63, 16, 0, 63, 16, 0,
  63, 17, 0, 63, 17, 0, 63, 18, 0, 63, 18, 0, 63, 19, 0, 63, 19, 0, 63, 20, 0, 63, 20, 0,
  63, 21, 0, 63, 21, 0, 63, 22, 0, 63, 22, 0, 63, 23, 0, 63, 24, 0, 63, 24, 0, 63, 25, 0,
  63, 25, 0, 63, 26, 0, 63, 26, 0, 63, 27, 0, 63, 27, 0, 63, 28, 0, 63, 28, 0, 63, 29, 0,
  63, 29, 0, 63, 30, 0, 63, 30, 0, 63, 31, 0, 63, 31, 0, 63, 32, 0, 63, 32, 0, 63, 33, 0,
  63, 33, 0, 63, 34, 0, 63, 34, 0, 63, 35, 0, 63, 35, 0, 63, 36, 0, 63, 36, 0, 63, 37, 0,
  63, 38, 0, 63, 38, 0, 63, 39, 0, 63, 39, 0, 63, 40, 0, 63, 40, 0, 63, 41, 0, 63, 41, 0,
  63, 42, 0, 63, 42, 0, 63, 43, 0, 63, 43, 0, 63, 44, 0, 63, 44, 0, 63, 45, 0, 63, 45, 0,
  63, 46, 0, 63, 46, 0, 63, 47, 0, 63, 47, 0, 63, 48, 0, 63, 48, 0, 63, 49, 0, 63, 49, 0,
  63, 50, 0, 63, 50, 0, 63, 51, 0, 63, 52, 0, 63, 52, 0, 63, 52, 0, 63, 52, 0, 63, 52, 0,
  63, 53, 0, 63, 53, 0, 63, 53, 0, 63, 53, 0, 63, 54, 0, 63, 54, 0, 63, 54, 0, 63, 54, 0,
  63, 54, 0, 63, 55, 0, 63, 55, 0, 63, 55, 0, 63, 55, 0, 63, 56, 0, 63, 56, 0, 63, 56, 0,
  63, 56, 0, 63, 57, 0, 63, 57, 0, 63, 57, 0, 63, 57, 0, 63, 57, 0, 63, 58, 0, 63, 58, 0,
  63, 58, 0, 63, 58, 0, 63, 59, 0, 63, 59, 0, 63, 59, 0, 63, 59, 0, 63, 60, 0, 63, 60, 0,
  63, 60, 0, 63, 60, 0, 63, 60, 0, 63, 61, 0, 63, 61, 0, 63, 61, 0, 63, 61, 0, 63, 62, 0,
  63, 62, 0, 63, 62, 0, 63, 62, 0, 63, 63, 0, 63, 63, 1, 63, 63, 2, 63, 63, 3, 63, 63, 4,
  63, 63, 5, 63, 63, 6, 63, 63, 7, 63, 63, 8, 63, 63, 9, 63, 63, 10, 63, 63, 10, 63, 63, 11,
  63, 63, 12, 63, 63, 13, 63, 63, 14, 63, 63, 15, 63, 63, 16, 63, 63, 17, 63, 63, 18, 63, 63, 19,
  63, 63, 20, 63, 63, 21, 63, 63, 21, 63, 63, 22, 63, 63, 23, 63, 63, 24, 63, 63, 25, 63, 63, 26,
  63, 63, 27, 63, 63, 28, 63, 63, 29, 63, 63, 30, 63, 63, 31, 63, 63, 31, 63, 63, 32, 63, 63, 33,
  63, 63, 34, 63, 63, 35, 63, 63, 36, 63, 63, 37, 63, 63, 38, 63, 63, 39, 63, 63, 40, 63, 63, 41,
  63, 63, 42, 63, 63, 42, 63, 63, 43, 63, 63, 44, 63, 63, 45, 63, 63, 46, 63, 63, 47, 63, 63, 48,
  63, 63, 49, 63, 63, 50, 63, 63, 51, 63, 63, 52, 63, 63, 52, 63, 63, 53, 63, 63, 54, 63, 63, 55,
  63, 63, 56, 63, 63, 57, 63, 63, 58, 63, 63, 59, 63, 63, 60, 63, 63, 61, 63, 63, 62, 63, 63, 63
};

/* --- Fire algorithm extracted from aafire.c --- */

static void
gentable(void)
{
    unsigned int i, p2;
    unsigned int minus = 800 / YSIZ;
    if (minus == 0)
        minus = 1;
    for (i = 0; i < MAXTABLE; i++)
    {
        if (i > minus)
        {
            p2 = (i - minus) / 5;
            table[i] = p2;
        }
        else
            table[i] = 0;
    }
}

#define MA 5

static void
firemain(void)
{
    register unsigned int i;
    unsigned char *p;
    i = 0;
#define END (bitmap + XSIZ * YSIZ)
    for (p = (unsigned char *)bitmap;
         (char *)p <= ((char *)(END));
         p += 1)
    {
        *p = table[(*(p + XSIZ - 1) + *(p + XSIZ + 1) + *(p + XSIZ)) +
                   (*(p + 2 * XSIZ - 1) + *(p + 2 * XSIZ + 1))];
    }
}

#define min(x, y) ((x) < (y) ? (x) : (y))

static void
drawfire(void)
{
    unsigned int i, last1, i1, i2;
    register unsigned char *p;

    height++;
    loop--;
    if (loop < 0)
        loop = rand() % 3, sloop++;
    i1 = 1;
    i2 = 4 * XSIZ + 1;
    for (p = (unsigned char *)bitmap + XSIZ * (YSIZ + 0);
         p < ((unsigned char *)bitmap + XSIZ * (YSIZ + 1));
         p++, i1 += 4, i2 -= 4)
    {
        last1 = rand() % min(i1, min(i2, height));
        i = rand() % 6;
        for (; p < (unsigned char *)bitmap + XSIZ * (YSIZ + 1) && i != 0;
             p++, i--, i1 += 4, i2 -= 4)
            *p = last1, last1 += rand() % 6 - 2, *(p + XSIZ) = last1, last1 +=
                rand() % 6 - 2;
        *(p + 2 * XSIZ) = last1, last1 += rand() % 6 - 2;
    }
    i = 0;
    firemain();

    caca_dither_bitmap(cv, 0, 0, caca_get_canvas_width(cv),
                       caca_get_canvas_height(cv), caca_dither, bitmap);
}

/* --- Internal init/teardown --- */

static void do_init(int cols, int rows)
{
    int i;
    uint32_t r[256], g[256], b[256], a[256];

    cv = caca_create_canvas(cols, rows);

    XSIZ = caca_get_canvas_width(cv) * 2;
    YSIZ = caca_get_canvas_height(cv) * 2 - 4;

    for (i = 0; i < 256; i++)
    {
        r[i] = pal[i * 3] * 64;
        g[i] = pal[i * 3 + 1] * 64;
        b[i] = pal[i * 3 + 2] * 64;
        a[i] = 0xfff;
    }

    caca_dither = caca_create_dither(8, XSIZ, YSIZ - 2, XSIZ, 0, 0, 0, 0);
    caca_set_dither_palette(caca_dither, r, g, b, a);

    bitmap = malloc(XSIZ * (YSIZ + 2));
    memset(bitmap, 0, XSIZ * (YSIZ + 2));

    gentable();
}

static void do_teardown(void)
{
    if (caca_dither)
    {
        caca_free_dither(caca_dither);
        caca_dither = NULL;
    }
    if (bitmap)
    {
        free(bitmap);
        bitmap = NULL;
    }
    if (cv)
    {
        caca_free_canvas(cv);
        cv = NULL;
    }
}

/* --- Exported WASM API --- */

EXPORT
void fire_init(int cols, int rows)
{
    prng_seed(42);
    height = 0;
    loop = 0;
    sloop = 0;
    do_init(cols, rows);
}

EXPORT
void fire_init_seeded(int cols, int rows, uint32_t seed)
{
    prng_seed(seed);
    height = 0;
    loop = 0;
    sloop = 0;
    do_init(cols, rows);
}

EXPORT
void fire_step(void)
{
    drawfire();
}

EXPORT
void fire_resize(int cols, int rows)
{
    uint32_t saved_state = _prng_state;
    do_teardown();
    _prng_state = saved_state;
    height = 0;
    loop = 0;
    sloop = 0;
    do_init(cols, rows);
}

EXPORT
int get_width(void)
{
    return caca_get_canvas_width(cv);
}

EXPORT
int get_height(void)
{
    return caca_get_canvas_height(cv);
}

EXPORT
const uint32_t *get_chars(void)
{
    return caca_get_canvas_chars(cv);
}

EXPORT
const uint32_t *get_attrs(void)
{
    return caca_get_canvas_attrs(cv);
}

EXPORT
void set_color_ansi(uint8_t fg, uint8_t bg)
{
    caca_set_color_ansi(cv, fg, bg);
}

EXPORT
uint32_t get_cur_attr(void)
{
    return caca_get_attr(cv, -1, -1);
}
