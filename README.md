# frequency_detector_wise_lab
This is a frequency detector that I got to run using the wasmtime engine on esp32c6_devkitc.  
Both projects were run on WSL Ubuntu with the following information:
- WSL version: 6.6.87.2-microsoft-standard-WSL2
- Ubuntu version: Ubuntu 24.04.3 LTS

Zephyr Version: 4.3.0
- Additional information included in the west-manifest.yml file

Rust Version: rustc 1.95.0-nightly (842bd5be2 2026-01-29)
Cargo Version: cargo 1.95.0-nightly (efcd9f586 2026-01-23)
rustup Information:
Default host: x86_64-unknown-linux-gnu
rustup home:  /home/jerryfen/.rustup

installed targets for active toolchain
--------------------------------------

aarch64-unknown-linux-gnu
riscv32i-unknown-none-elf
riscv32imac-unknown-none-elf
riscv64gc-unknown-linux-gnu
riscv64imac-unknown-none-elf
wasm32-unknown-unknown
wasm32-wasip1
wasm32-wasip2
x86_64-unknown-linux-gnu

active toolchain
----------------

nightly-x86_64-unknown-linux-gnu (default)
rustc 1.95.0-nightly (842bd5be2 2026-01-29)


## ESP32C6 Devkitc
The esp32 directory was placed under the zephyrproject/zephyr/apps/ directory (I created the "apps" directory under the zephyr directory myself).


### Pins for ESP32:
Setup one of the adc capable GPIO pins in the board overlay file.  Also the secondary uart is necessary for transmitting rr bytes back to 
the laptop host device.  Here is the GPIO setup used in the project currently pushed to git:

GPIO3: ADC Pin
GPIO4 => Connect to RX on USB to UART Cable
GPIO5 => Connect to TX on USB to UART Cable

### Workflow
- Navigate to the "esp32/adc_embed" directory and run the following command to build the .wasm file:
<pre> ```clang --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--export-all -o wasm_component/adc.wasm src/kiss_fft.c src/main.c src/wasm_libc.c``` </pre>

  - We have a shim.h, which is mainly to get the compiler to actually compile the code instead of complaining about undefined functions.  Broadly speaking, the general pattern is that we want any device specific functions (i.e. any peripheral function like "adc_read_dt") should be put in the interface (.wit file) and we keep everything else in application code.
  - I did not use wit bindgen to generate the .wit file because for printk, I did not like having to deal with what the bindgen created, and prefered to do something a bit more hacky (take in the string, and pass it to the rust side code.  Then in the rust side code pass the string pointer and length to the actual extern c implementations)

- Run the following command to embed the "adc.wit" interface into the "adc.wasm" file to produce the new "adc_embed.wasm" file:
<pre> ```wasm-tools component embed adc.wit wasm_component/adc.wasm -o wasm_component/adc_embed.wasm``` </pre>

- Run the following command to create a new component out of the "adc_embed.wasm" file with the component as the filw "adc.component.wasm":
<pre> ```wasm-tools component new wasm_component/adc_embed.wasm -o wasm_component/adc.component.wasm``` </pre>

- Navigate to the "esp32/adc_pulley_embed/wasmtime-rr-prototyping/target/debug/" directory and run the "pulley_exp_wasmtime_with_std" executable to create the "adc.rr.cwasm" file.  This .cwasm file will be embedded as bytes in the rust side code and will be run with pulley32.

- Navigate to the "esp32/adc_pulley_embed/wasmtime-rr-prototyping/adc_embed_nostd/" directory and then run the following command:
<pre> ```RUSTFLAGS="-C link-arg=--initial-memory=65536 -C link-arg=--stack-first -C link-arg=-zstack-size=4096"   cargo build --release --target riscv32imac-unknown-none-elf``` </pre>

Note all of this can be done automatically for you by running <pre> ```./script.sh``` </pre> in the "esp32/adc_pulley_embed" directory.

- Navigate to the "esp32/adc_pulley_embed" directory and run the following commands to build and then flash the esp32:
<pre> ```
west build -b esp32c6_devkitc . -p always -- -DDTC_OVERLAY_FILE=boards/esp32c6_devkitc.overlay
west flash
``` </pre>

From here you should be able to see the frequencies being printed out by running <pre> ```west espressif monitor``` </pre>.  

For record replay follow the preceding instructions:

- Open 3 terminals and navigate all of them to the "esp32/adc_pulley_embed" directory

- In 1 terminal in the ~/zephyrproject/zephyr/apps/adc_pulley_embed run:
<pre> ```python3 fifo_proc/write_fifo.py``` </pre>

- In another terminal run:
<pre> ```WASMTIME_LOG=debug &lt;path to wasmtime executable&gt; replay --trace /tmp/uart_data -d 4096 esp32/adc_embed/wasm_component/adc.component.wasm``` </pre>

- Now in the final terminal run <pre> ```west flash``` </pre>.  You should see record replay continuously running in the terminal running wasmtime

The python script waits for a "magic sequence" that gets printed in the rust code before transmitting bytes to the record replay.  Feel free to modify to your liking.

## Raspberry Pi Zero 2 W

### Pins for Raspberry Pi Zero 2 W
Vdd => Arduino1
Gnd => Arduino6
L/R => Arduino6
Sck => Arduino12 = GPIO18
WS  => Arduino35 = GPIO19
SD  => Arduino38 = GPIO20

### Testing Without Wasmtime

freq_detect_test/src/ holds the code for the application code without wasmtime. 
This is done to isolate wasmtime effects from application code effects.  Compile with clang and run the binary.
It should spit out frequencies at you, every so often it will print "overrun", which means the samples are not draining 
fast enough, and that we ran into a DMA buffer overrun.  The code should handle this by resetting the peripheral, 
overruns are expected, as long as it still gives you good samples after "overrun" prints it is OK.

### Workflow with Wasmtime

- Enter the "rpi/freq_detect_wasm/freq_detect_embed/" directory.
  - We have a shim.h which is mainly to get the compiler to actually compile the code instead of complaining about undefined functions.  Broadly speaking, the general pattern is that we want any device specific functions (i.e. any peripheral function like "snd_pcm_prepare") should be put in the interface (.wit file) and we keep everything else in application code.
  - I did not use wit bindgen to generate the .wit file because for printf, I did not like having to deal with what the bindgen created, and prefered to do something a bit more hacky (take in the string, and pass it to the rust side code.  Then in the rust side code pass the string pointer and length to the actual extern c implementations) 

- Compile everything into a "tuner.wasm" file in the directory using the following command:
<pre> ```
  clang \
  --target=wasm32 \
  -O2 \
  -ffast-math \
  -nostdlib \
  -Wl,--no-entry \
  -Wl,--export=run \
  -Wl,--stack-first \
  -Wl,--initial-memory=1048576 \
  -o wasm_component/tuner.wasm \
  src/main.c src/kiss_fft.c src/wasm_libc.c
``` </pre>

- Run the following command to embed the "adc.wit" interface into the generated "tuner.wasm" file to create a new "tuner_embed.wasm" file
<pre> ```wasm-tools component embed adc.wit wasm_component/tuner.wasm -o wasm_component/tuner_embed.wasm``` </pre>

- Run the following command to create a new component out of "tuner_embed.wasm" which is in a new "tuner.component.wasm"
<pre> ```wasm-tools component new wasm_component/tuner_embed.wasm -o wasm_component/tuner.component.wasm``` </pre>
  - wit_gen directory is to be ignored, feel free to delete it.   

- First we need to setup our system so we can cross=compile for the Rpi.  Run the following:
<pre> ```
sudo apt update
sudo apt install -y gcc-aarch64-linux-gnu debootstrap qemu-user-static
``` </pre>

- Add the Rust target by navigating to the "rpi/freq_detect_wasm/freq_detect_pulley_embed/wasmtime-rr-prototyping" directory and then running the following command:
<pre> ```rustup target add aarch64-unknown-linux-gnu``` </pre>

- Create a new sysroot for all of the ARM stuff:
<pre> ```sudo debootstrap --foreign --arch=arm64 noble sysroot http://ports.ubuntu.com/ubuntu-ports``` </pre>

- Copy QEMU so the existing OS can run ARM64 binaries on setup:
<pre> ```sudo cp /usr/bin/qemu-aarch64-static sysroot/usr/bin/``` </pre>

- Complete the second stage:
<pre> ```sudo chroot sysroot /debootstrap/debootstrap --second-stage``` </pre>

- Install ARM64 ALSA development libraries inside the sysroot so that we can link them into the executable:
<pre> ```
sudo chroot sysroot apt update
sudo chroot sysroot apt install -y libasound2-dev
``` </pre>

- Give Cargo read permissions of the new sysroot:
<pre> ```sudo chown -R $USER:$USER sysroot``` </pre>

- Configure Cargo to compile for ARM64, navigate to the "rpi/freq_detect_wasm/freq_detect_pulley_embed/wasmtime-rr-prototyping/pulley_exp_wasmtime_nostd/" directory and run <pre> ```mkdir -p .cargo``` </pre> and then in the .cargo hidden directory create a "config.toml" file and put the following inside:
<pre> ```
[build]
target = "aarch64-unknown-linux-gnu"

[target.aarch64-unknown-linux-gnu]
linker = "aarch64-linux-gnu-gcc"
rustflags = [
    "-C", "link-arg=--sysroot=&lt;path to sysroot directory&gt;",
]
``` </pre>

- Navigate back to the "rpi/freq_detect_wasm/freq_detect_pulley_embed/wasmtime-rr-prototyping/pulley_exp_wasmtime_nostd/" directory and create a "build.rs" file and put the following inside:
<pre> ```
fn main() {
    println!("cargo:rerun-if-changed=src/bridge.c");
    println!("cargo:rerun-if-changed=build.rs");

    cc::Build::new()
        .file("src/bridge.c")
        .compiler("aarch64-linux-gnu-gcc")
        .flag("--sysroot=&lt;path to sysroot&gt;")
        .compile("bridge");

    println!("cargo:rustc-link-search=native=&lt;path to sysroot&gt;/usr/lib/aarch64-linux-gnu");
    println!("cargo:rustc-link-lib=asound");
}
``` </pre>

- Navigate to the "rpi/freq_detect_wasm/freq_detect_pulley_embed/wasmtime-rr-prototyping/pulley_exp_wasmtime_nostd/" directory (name is misleading, sorry) and run the following command to build the executable which you will SCP to the rpi:
<pre> ```cargo build --target aarch64-unknown-linux-gnu -v``` </pre>

- In the "rpi/freq_detect_wasm/freq_detect_pulley_embed/wasmtime-rr-prototyping/target/aarch64-unknown-linux-gnu/debug" directory there should be an "rpi_embed" executable.  SCP this to the RPi Zero 2 using your favorite method (I used winSCP).

- SSH into the RPi using your favorite method (I used mobaXterm), and then run the executable.




