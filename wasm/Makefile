LIBCACA_DIR = ../libcaca/caca

CACA_SOURCES = \
	$(LIBCACA_DIR)/canvas.c \
	$(LIBCACA_DIR)/dither.c \
	$(LIBCACA_DIR)/attr.c \
	$(LIBCACA_DIR)/string.c \
	$(LIBCACA_DIR)/charset.c \
	$(LIBCACA_DIR)/dirty.c \
	$(LIBCACA_DIR)/frame.c \
	$(LIBCACA_DIR)/time.c

FIRE_SOURCE = cacafire_wasm.c

CFLAGS_COMMON = -O2 -I. -I$(LIBCACA_DIR) -DLIBCACA

# --- WASM target ---

EMCC = emcc
WASM_OUT = ../build/cacafire.js

EXPORTED_FUNCTIONS = \
	_fire_init,\
	_fire_init_seeded,\
	_fire_step,\
	_fire_resize,\
	_get_width,\
	_get_height,\
	_get_chars,\
	_get_attrs,\
	_set_color_ansi,\
	_get_cur_attr,\
	_malloc,\
	_free

EMCC_FLAGS = \
	$(CFLAGS_COMMON) \
	-s WASM=1 \
	-s MODULARIZE=1 \
	-s EXPORT_NAME="CacafireModule" \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s NO_EXIT_RUNTIME=1 \
	-s ENVIRONMENT=web,node \
	-s "EXPORTED_FUNCTIONS=[$(EXPORTED_FUNCTIONS)]" \
	-s "EXPORTED_RUNTIME_METHODS=['HEAPU32']"

# --- Native target (golden reference generator) ---

CC = cc
GEN_REF_OUT = ../build/gen_reference

# --- Targets ---

.PHONY: all wasm native clean

all: wasm native

wasm: $(WASM_OUT)

$(WASM_OUT): $(FIRE_SOURCE) $(CACA_SOURCES) config.h prng.h
	@mkdir -p ../build
	$(EMCC) $(EMCC_FLAGS) $(FIRE_SOURCE) $(CACA_SOURCES) -o $(WASM_OUT)

native: $(GEN_REF_OUT)

$(GEN_REF_OUT): $(FIRE_SOURCE) $(CACA_SOURCES) ../test/gen_reference.c config.h prng.h
	@mkdir -p ../build
	$(CC) $(CFLAGS_COMMON) ../test/gen_reference.c $(FIRE_SOURCE) $(CACA_SOURCES) -o $(GEN_REF_OUT)

clean:
	rm -f $(WASM_OUT) ../build/cacafire.wasm $(GEN_REF_OUT)
