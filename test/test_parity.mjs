/*
 * test_parity.mjs - WASM vs native byte comparison
 *
 * Loads the WASM module, runs the fire simulation with the same seed as
 * the native golden reference, and byte-compares output at checkpoint frames.
 */

import { readFileSync } from 'fs';
import { createRequire } from 'module';
import { fileURLToPath } from 'url';
import { dirname, join } from 'path';

const __dirname = dirname(fileURLToPath(import.meta.url));
const rootDir = join(__dirname, '..');

// Load the Emscripten module
const require = createRequire(import.meta.url);
const CacafireModule = require(join(rootDir, 'build', 'cacafire.js'));

const CHECKPOINT_FRAMES = [1, 10, 50, 100];
const COLS = 80;
const ROWS = 32;
const SEED = 42;

async function main() {
    const Module = await CacafireModule();

    Module._fire_init_seeded(COLS, ROWS, SEED);

    const w = Module._get_width();
    const h = Module._get_height();
    const size = w * h;

    console.log(`Testing WASM parity: ${w}x${h} canvas, seed=${SEED}`);

    let frame = 0;
    let ci = 0;
    let passed = 0;
    let failed = 0;

    while (ci < CHECKPOINT_FRAMES.length) {
        frame++;
        Module._fire_step();

        if (frame === CHECKPOINT_FRAMES[ci]) {
            const charsPtr = Module._get_chars() >>> 2;
            const attrsPtr = Module._get_attrs() >>> 2;

            const wasmChars = new Uint32Array(
                Module.HEAPU32.buffer.slice(charsPtr * 4, (charsPtr + size) * 4)
            );
            const wasmAttrs = new Uint32Array(
                Module.HEAPU32.buffer.slice(attrsPtr * 4, (attrsPtr + size) * 4)
            );

            const goldenCharsPath = join(rootDir, 'test', 'golden', `frame_${String(frame).padStart(3, '0')}_chars.bin`);
            const goldenAttrsPath = join(rootDir, 'test', 'golden', `frame_${String(frame).padStart(3, '0')}_attrs.bin`);

            let goldenChars, goldenAttrs;
            try {
                goldenChars = new Uint32Array(readFileSync(goldenCharsPath).buffer);
                goldenAttrs = new Uint32Array(readFileSync(goldenAttrsPath).buffer);
            } catch (e) {
                console.error(`  Frame ${frame}: FAIL - cannot read golden file: ${e.message}`);
                failed++;
                ci++;
                continue;
            }

            let charsDiff = 0;
            let attrsDiff = 0;

            for (let i = 0; i < size; i++) {
                if (wasmChars[i] !== goldenChars[i]) charsDiff++;
                if (wasmAttrs[i] !== goldenAttrs[i]) attrsDiff++;
            }

            if (charsDiff === 0 && attrsDiff === 0) {
                console.log(`  Frame ${frame}: PASS`);
                passed++;
            } else {
                console.error(`  Frame ${frame}: FAIL - ${charsDiff} char diffs, ${attrsDiff} attr diffs out of ${size}`);
                /* Show first few diffs */
                let shown = 0;
                for (let i = 0; i < size && shown < 5; i++) {
                    if (wasmChars[i] !== goldenChars[i]) {
                        console.error(`    chars[${i}]: wasm=0x${wasmChars[i].toString(16)} golden=0x${goldenChars[i].toString(16)}`);
                        shown++;
                    }
                }
                shown = 0;
                for (let i = 0; i < size && shown < 5; i++) {
                    if (wasmAttrs[i] !== goldenAttrs[i]) {
                        console.error(`    attrs[${i}]: wasm=0x${wasmAttrs[i].toString(16)} golden=0x${goldenAttrs[i].toString(16)}`);
                        shown++;
                    }
                }
                failed++;
            }
            ci++;
        }
    }

    console.log(`\nResults: ${passed} passed, ${failed} failed`);
    process.exit(failed > 0 ? 1 : 0);
}

main().catch(e => {
    console.error(e);
    process.exit(1);
});
