/*
 * Portable deterministic PRNG (xorshift32).
 * Ensures byte-identical output between native and WASM builds,
 * since Emscripten's musl rand() differs from macOS rand().
 */

#ifndef __PRNG_H__
#define __PRNG_H__

#include <stdint.h>

static uint32_t _prng_state = 42;

static inline void prng_seed(uint32_t s)
{
    _prng_state = s ? s : 1;
}

static inline int prng_rand(void)
{
    _prng_state ^= _prng_state << 13;
    _prng_state ^= _prng_state >> 17;
    _prng_state ^= _prng_state << 5;
    return (int)(_prng_state & 0x7fffffff);
}

#endif /* __PRNG_H__ */
