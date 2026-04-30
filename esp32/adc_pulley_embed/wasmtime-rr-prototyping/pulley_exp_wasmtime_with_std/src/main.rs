use wasmtime::*;
use std::fs;
use std::error::Error;

fn main() -> Result<(), Box<dyn Error>> {
    let mut config = Config::new();
    // config.wasm_gc(false);
    // config.gc_support(false);
    // config.target("pulley32").unwrap();
    // config.memory_init_cow(false);
    // config.strategy(Strategy::Cranelift);
    // config.signals_based_traps(false);
    // // for use with blinky_embed only
    // config.memory_reservation(0);
    // config.memory_reservation_for_growth(0);
    // config.memory_guard_size(0);

    // for use with rr
    config.gc_support(false);
    config.target("pulley32").unwrap();
    config.memory_init_cow(false);
    config.signals_based_traps(false);
    config.wasm_component_model(true);
    config.max_wasm_stack(16 * 1024);
    config.memory_reservation(0);
    config.memory_reservation_for_growth(0);
    config.memory_guard_size(0);
    // config.debug_info(true);
    // config.relaxed_simd_deterministic(false);
    config.rr(RRConfig::Recording);
    // config.rr(RRConfig::None);

    let engine = Engine::new(&config)?;

    // 1. Read wasm from file
    // let wasm = fs::read("/home/jerryfen/add.wasm")?;
    // let wasm = fs::read("/home/jerryfen/wasmtime-rr-prototyping/pulley_exp_wasmtime_with_std/divide_prog/divide.wasm")?;
    // let wasm = fs::read("/home/jerryfen/zephyrproject/zephyr/apps/blinky/wasm_module/blinky.wasm")?;

    // let wasm = fs::read("/home/jerryfen/zephyrproject/zephyr/apps/blinky/wasm_component/blinky.component.wasm")?;
    // let wasm = fs::read("/home/jerryfen/zephyrproject/zephyr/apps/blinky_5times/wasm_component/blinky_5times.component.wasm")?;
    // let wasm = fs::read("/home/jerryfen/zephyrproject/zephyr/apps/blinky_5times/blink101_wasm_component/blink101.component.wasm")?;
    let wasm = fs::read("/home/jerryfen/zephyrproject/zephyr/apps/adc_embed/wasm_component/adc.component.wasm")?;


    // let result = Engine::precompile_module(&engine, &wasm).unwrap();
    
    // for blinky component
    // engine::new on the raw wasm file, not the cwasm
    let result = Engine::precompile_component(&engine, &wasm).unwrap();

    // // 4. Save to file
    // fs::write("./add.cwasm", result)?;
    // fs::write("/home/jerryfen/wasmtime-rr-prototyping/pulley_exp_wasmtime_with_std/divide_prog/divide.cwasm", result)?;
    // fs::write("/home/jerryfen/zephyrproject/zephyr/apps/blinky/wasm_component/blinky.component.rr.cwasm", result)?;
    // fs::write("/home/jerryfen/zephyrproject/zephyr/apps/blinky_5times/wasm_component/blinky_5times.component.rr.cwasm", result)?;
    // fs::write("/home/jerryfen/zephyrproject/zephyr/apps/blinky_5times/blink101_wasm_component/blink101.component.rr.cwasm", result)?;
    fs::write("/home/jerryfen/zephyrproject/zephyr/apps/adc_embed/wasm_component/adc.rr.cwasm", result)?;

    println!("Serialized module written to add.cwasm");
    Ok(())
}
