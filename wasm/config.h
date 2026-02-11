/*
 * Minimal config.h for building libcaca with Emscripten (or natively).
 * No display drivers — only the dithering engine + canvas.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define HAVE_ERRNO_H       1
#define HAVE_VSNPRINTF     1
#define HAVE_GETTIMEOFDAY  1
#define HAVE_USLEEP        1
#define HAVE_SYS_TIME_H   1
#define HAVE_UNISTD_H     1

/* No display drivers */
/* #undef USE_NCURSES */
/* #undef USE_SLANG   */
/* #undef USE_X11     */
/* #undef USE_GL      */
/* #undef USE_WIN32   */
/* #undef USE_CONIO   */
/* #undef USE_COCOA   */
/* #undef USE_VGA     */

/* No visibility attributes needed */
/* #undef CACA_ENABLE_VISIBILITY */

#define VERSION "0.99.beta20"
#define PACKAGE "caca"

#endif /* __CONFIG_H__ */
