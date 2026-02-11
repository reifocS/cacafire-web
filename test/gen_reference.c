/*
 * gen_reference.c - Generate golden reference data for parity testing.
 *
 * Compiled natively (not with emcc) using the same cacafire_wasm.c + libcaca
 * sources. Runs the fire simulation with a fixed PRNG seed and dumps
 * chars[]/attrs[] at checkpoint frames.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* We reuse the WASM API functions compiled natively */
extern void fire_init_seeded(int cols, int rows, uint32_t seed);
extern void fire_step(void);
extern int get_width(void);
extern int get_height(void);
extern const uint32_t *get_chars(void);
extern const uint32_t *get_attrs(void);

static const int CHECKPOINT_FRAMES[] = { 1, 10, 50, 100 };
#define NUM_CHECKPOINTS (sizeof(CHECKPOINT_FRAMES) / sizeof(CHECKPOINT_FRAMES[0]))

static int dump_buffer(const char *path, const uint32_t *buf, int count)
{
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "ERROR: cannot open %s for writing\n", path);
        return -1;
    }
    fwrite(buf, sizeof(uint32_t), count, f);
    fclose(f);
    return 0;
}

int main(void)
{
    int cols = 80, rows = 32;
    uint32_t seed = 42;
    char path[256];
    int frame = 0;
    int ci = 0; /* checkpoint index */

    fire_init_seeded(cols, rows, seed);

    int w = get_width();
    int h = get_height();
    int size = w * h;

    printf("Generating golden reference: %dx%d canvas, seed=%u\n", w, h, seed);

    while (ci < (int)NUM_CHECKPOINTS) {
        frame++;
        fire_step();

        if (frame == CHECKPOINT_FRAMES[ci]) {
            snprintf(path, sizeof(path), "test/golden/frame_%03d_chars.bin", frame);
            if (dump_buffer(path, get_chars(), size) < 0) return 1;

            snprintf(path, sizeof(path), "test/golden/frame_%03d_attrs.bin", frame);
            if (dump_buffer(path, get_attrs(), size) < 0) return 1;

            printf("  Frame %3d: dumped %d uint32 values\n", frame, size);
            ci++;
        }
    }

    printf("Done. Golden files in test/golden/\n");
    return 0;
}
