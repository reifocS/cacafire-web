/*
 * app.js - JS rendering layer for cacafire-web
 *
 * Loads the WASM module, runs the fire simulation, and renders
 * colored ASCII characters to an HTML5 Canvas.
 */

/* ANSI color palette (matches ansitab16 in libcaca/caca/attr.c)
 * ansitab16 values are 0xfRGB, each nibble * 0x11 → 8-bit color */
const ANSI_COLORS = [
    '#000000', // 0  black
    '#0000aa', // 1  blue
    '#00aa00', // 2  green
    '#00aaaa', // 3  cyan
    '#aa0000', // 4  red
    '#aa00aa', // 5  magenta
    '#aa5500', // 6  brown
    '#aaaaaa', // 7  light gray
    '#555555', // 8  dark gray
    '#5555ff', // 9  light blue
    '#55ff55', // 10 light green
    '#55ffff', // 11 light cyan
    '#ff5555', // 12 light red
    '#ff55ff', // 13 light magenta
    '#ffff55', // 14 yellow
    '#ffffff', // 15 white
];

/**
 * Decode a libcaca attribute value into fg/bg ANSI color indices.
 * Encoding: attr = ((bg | 0x40) << 18) | ((fg | 0x40) << 4)
 */
function decodeAttr(attr) {
    const fg = ((attr >>> 4) & 0x3fff) ^ 0x40;
    const bg = ((attr >>> 18) & 0x3fff) ^ 0x40;
    return { fg, bg };
}

/* Make decodeAttr and ANSI_COLORS available for testing */
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { decodeAttr, ANSI_COLORS };
}

(async function main() {
    /* Only run in browser */
    if (typeof window === 'undefined') return;

    const canvas = document.getElementById('fire');
    const ctx = canvas.getContext('2d');

    const Module = await CacafireModule();

    let paused = false;
    let animId = null;

    function setup() {
        const dpr = window.devicePixelRatio || 1;
        const vw = window.innerWidth;
        const vh = window.innerHeight;

        /* Scale cell size with DPR for sharp rendering */
        const cellH = Math.max(10, Math.round(16 * dpr));
        const cellW = Math.round(cellH * 0.6);

        const cols = Math.max(20, Math.floor((vw * dpr) / cellW));
        const rows = Math.max(10, Math.floor((vh * dpr) / cellH));

        canvas.width = cols * cellW;
        canvas.height = rows * cellH;

        const fontSize = Math.round(cellH * 0.85);
        ctx.font = fontSize + 'px monospace';
        ctx.textBaseline = 'top';

        return { cols, rows, cellW, cellH, dpr };
    }

    let state = setup();
    Module._fire_init(state.cols, state.rows);

    function render() {
        if (paused) {
            animId = requestAnimationFrame(render);
            return;
        }

        Module._fire_step();

        const w = Module._get_width();
        const h = Module._get_height();
        const charsPtr = Module._get_chars() >>> 2; /* byte offset → uint32 index */
        const attrsPtr = Module._get_attrs() >>> 2;
        const chars = Module.HEAPU32.subarray(charsPtr, charsPtr + w * h);
        const attrs = Module.HEAPU32.subarray(attrsPtr, attrsPtr + w * h);

        const cellW = state.cellW;
        const cellH = state.cellH;

        /* Clear canvas */
        ctx.fillStyle = '#000000';
        ctx.fillRect(0, 0, canvas.width, canvas.height);

        /* Batch rendering by color: first backgrounds, then foregrounds */

        /* Draw background rectangles */
        let lastBg = -1;
        for (let y = 0; y < h; y++) {
            const rowOff = y * w;
            for (let x = 0; x < w; x++) {
                const attr = attrs[rowOff + x];
                const bg = ((attr >>> 18) & 0x3fff) ^ 0x40;
                if (bg > 0 && bg < 16) {
                    if (bg !== lastBg) {
                        ctx.fillStyle = ANSI_COLORS[bg];
                        lastBg = bg;
                    }
                    ctx.fillRect(x * cellW, y * cellH, cellW, cellH);
                }
            }
        }

        /* Draw foreground characters */
        let lastFg = -1;
        for (let y = 0; y < h; y++) {
            const rowOff = y * w;
            for (let x = 0; x < w; x++) {
                const ch = chars[rowOff + x];
                if (ch <= 0x20) continue; /* skip spaces and control chars */

                const attr = attrs[rowOff + x];
                const fg = ((attr >>> 4) & 0x3fff) ^ 0x40;
                if (fg !== lastFg) {
                    ctx.fillStyle = ANSI_COLORS[fg < 16 ? fg : 7];
                    lastFg = fg;
                }
                ctx.fillText(String.fromCodePoint(ch), x * cellW, y * cellH);
            }
        }

        animId = requestAnimationFrame(render);
    }

    animId = requestAnimationFrame(render);

    /* Resize handling with debounce */
    let resizeTimer = null;
    window.addEventListener('resize', function () {
        clearTimeout(resizeTimer);
        resizeTimer = setTimeout(function () {
            state = setup();
            Module._fire_resize(state.cols, state.rows);
        }, 150);
    });

    /* Space to toggle pause */
    window.addEventListener('keydown', function (e) {
        if (e.code === 'Space') {
            e.preventDefault();
            paused = !paused;
        }
    });
})();
