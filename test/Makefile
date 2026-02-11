NATIVE_BIN = ../build/cacafire_native
GEN_REF = gen_reference.c

LIBCACA_DIR = ../libcaca/caca
WASM_DIR = ../wasm

CACA_SOURCES = \
	$(LIBCACA_DIR)/canvas.c \
	$(LIBCACA_DIR)/dither.c \
	$(LIBCACA_DIR)/attr.c \
	$(LIBCACA_DIR)/string.c \
	$(LIBCACA_DIR)/charset.c \
	$(LIBCACA_DIR)/dirty.c \
	$(LIBCACA_DIR)/frame.c \
	$(LIBCACA_DIR)/time.c

CC = cc
CFLAGS = -O2 -I$(WASM_DIR) -I$(LIBCACA_DIR) -DLIBCACA

GEN_BIN = ../build/gen_reference

.PHONY: golden clean

golden: $(GEN_BIN)
	@mkdir -p golden
	cd .. && ./build/gen_reference

$(GEN_BIN): $(GEN_REF) $(WASM_DIR)/cacafire_wasm.c $(CACA_SOURCES)
	@mkdir -p ../build
	$(CC) $(CFLAGS) $(GEN_REF) $(WASM_DIR)/cacafire_wasm.c $(CACA_SOURCES) -o $(GEN_BIN)

clean:
	rm -f $(GEN_BIN)
	rm -rf golden
