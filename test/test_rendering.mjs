/*
 * test_rendering.mjs - JS attr decoding verification
 *
 * Loads the WASM module, tests all 16x16 fg/bg ANSI color combinations,
 * and verifies the JS decodeAttr() function correctly decodes them.
 * Also verifies the ANSI_COLORS lookup table matches ansitab16.
 */

import { createRequire } from 'module';
import { fileURLToPath } from 'url';
import { dirname, join } from 'path';

const __dirname = dirname(fileURLToPath(import.meta.url));
const rootDir = join(__dirname, '..');

const require = createRequire(import.meta.url);
const CacafireModule = require(join(rootDir, 'build', 'cacafire.js'));

/* decodeAttr and ANSI_COLORS from app.js (duplicated here for testing) */
function decodeAttr(attr) {
    const fg = ((attr >>> 4) & 0x3fff) ^ 0x40;
    const bg = ((attr >>> 18) & 0x3fff) ^ 0x40;
    return { fg, bg };
}

const ANSI_COLORS = [
    '#000000', '#0000aa', '#00aa00', '#00aaaa',
    '#aa0000', '#aa00aa', '#aa5500', '#aaaaaa',
    '#555555', '#5555ff', '#55ff55', '#55ffff',
    '#ff5555', '#ff55ff', '#ffff55', '#ffffff',
];

/* Expected colors from ansitab16 (0xfRGB → #RRGGBB) */
const ANSITAB16 = [
    0xf000, 0xf00a, 0xf0a0, 0xf0aa, 0xfa00, 0xfa0a, 0xfa50, 0xfaaa,
    0xf555, 0xf55f, 0xf5f5, 0xf5ff, 0xff55, 0xff5f, 0xfff5, 0xffff,
];

function rgb4to8(val) {
    return (val * 0x11).toString(16).padStart(2, '0');
}

function ansitabToHex(val) {
    const r = (val >> 8) & 0xf;
    const g = (val >> 4) & 0xf;
    const b = val & 0xf;
    return '#' + rgb4to8(r) + rgb4to8(g) + rgb4to8(b);
}

async function main() {
    const Module = await CacafireModule();

    /* Initialize a small canvas just for testing attr encoding */
    Module._fire_init(10, 10);

    let passed = 0;
    let failed = 0;

    console.log('Testing attr decoding for all 16x16 fg/bg combinations...');

    for (let fg = 0; fg < 16; fg++) {
        for (let bg = 0; bg < 16; bg++) {
            Module._set_color_ansi(fg, bg);
            const attr = Module._get_cur_attr();
            const decoded = decodeAttr(attr);

            if (decoded.fg !== fg || decoded.bg !== bg) {
                console.error(`  FAIL: fg=${fg} bg=${bg} → attr=0x${attr.toString(16)} → decoded fg=${decoded.fg} bg=${decoded.bg}`);
                failed++;
            } else {
                passed++;
            }
        }
    }

    console.log(`  Attr decoding: ${passed} passed, ${failed} failed`);

    /* Test ANSI_COLORS table matches ansitab16 */
    console.log('\nTesting ANSI_COLORS table against ansitab16...');
    let colorPassed = 0;
    let colorFailed = 0;

    for (let i = 0; i < 16; i++) {
        const expected = ansitabToHex(ANSITAB16[i]);
        const actual = ANSI_COLORS[i];
        if (actual !== expected) {
            console.error(`  FAIL: ANSI_COLORS[${i}] = ${actual}, expected ${expected}`);
            colorFailed++;
        } else {
            colorPassed++;
        }
    }

    console.log(`  Color table: ${colorPassed} passed, ${colorFailed} failed`);

    const totalFailed = failed + colorFailed;
    console.log(`\nResults: ${passed + colorPassed} passed, ${totalFailed} failed`);
    process.exit(totalFailed > 0 ? 1 : 0);
}

main().catch(e => {
    console.error(e);
    process.exit(1);
});
