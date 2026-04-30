cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tuner_pulley_embed)

# target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/main.c wasmtime-rr-prototyping/adc_embed_nostd/src/bridge.c)

# ---- Rust integration ----
set(RUST_CRATE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wasmtime-rr-prototyping/adc_embed_nostd)
set(RUST_TARGET riscv32imac-unknown-none-elf)
set(RUST_PROFILE release)

# Update this if your crate name differs:
set(RUST_LIB ${CMAKE_CURRENT_SOURCE_DIR}/wasmtime-rr-prototyping/target/${RUST_TARGET}/${RUST_PROFILE}/libadc_embed_nostd.a)


target_link_libraries(app PUBLIC "${RUST_LIB}")

# 2. The Custom Command to actually run Cargo
add_custom_command(
    OUTPUT "${RUST_LIB_PATH}"
    COMMAND cargo build --release --target riscv32imac-unknown-none-elf
    WORKING_DIRECTORY "${RUST_SOURCE_DIR}"
    COMMENT "Generating Rust Static Library via Cargo..."
    VERBATIM
)

# 3. Create a custom target so CMake has something to track
add_custom_target(run_cargo_build DEPENDS "${RUST_LIB_PATH}")

# 4. Create an interface library to hold the Rust archive
add_library(rust_app_lib INTERFACE)
target_link_libraries(rust_app_lib INTERFACE "${RUST_LIB_PATH}")


