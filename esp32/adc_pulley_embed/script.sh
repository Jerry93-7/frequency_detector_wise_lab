#!/bin/bash
set -euo pipefail

# ---------------------------------------------------------------------------
# script.sh
# ---------------------------------------------------------------------------

# --- your commands go here ---

cd ..

cd adc_embed/

# clang --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--export-all -o wasm_component/adc.wasm src/kiss_fft.c src/main.c src/wasm_libc.c

clang --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--export-all -o wasm_component/adc.wasm src/kiss_fft.c src/main.c src/wasm_libc.c

wasm-tools component embed adc.wit wasm_component/adc.wasm -o wasm_component/adc_embed.wasm

wasm-tools component new wasm_component/adc_embed.wasm -o wasm_component/adc.component.wasm

cd .. 

cd adc_pulley_embed/wasmtime-rr-prototyping/

./target/debug/pulley_exp_wasmtime_with_std

cd adc_embed_nostd/

RUSTFLAGS="-C link-arg=--initial-memory=65536 -C link-arg=--stack-first -C link-arg=-zstack-size=4096"   cargo build --release --target riscv32imac-unknown-none-elf


