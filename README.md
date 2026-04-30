# frequency_detector_wise_lab
This is a frequency detector that I got to run using the wasmtime engine on esp32c6_devkitc.  

## ESP32C6 Devkitc
The esp32 directory was placed under the zephyrproject/zephyr/apps/ directory (I created the "apps" directory under the zephyr directory myself).

### Pins for ESP32:
Setup one of the adc capable GPIO pins in the board overlay file.  Also the secondary uart is necessary for transmitting rr bytes back to 
the laptop host device.  Any WIFI stuff in bridge.c did not work out and can be removed.

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
    "-C", "link-arg=--sysroot=`<path to sysroot directory>`",
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
        .flag("--sysroot=`<path to sysroot>`")
        .compile("bridge");

    println!("cargo:rustc-link-search=native=`<path to sysroot>`/usr/lib/aarch64-linux-gnu");
    println!("cargo:rustc-link-lib=asound");
}
``` </pre>

- Navigate to the "rpi/freq_detect_wasm/freq_detect_pulley_embed/wasmtime-rr-prototyping/pulley_exp_wasmtime_nostd/" directory (name is misleading, sorry) and run the following command to build the executable which you will SCP to the rpi:
<pre> ```cargo build --target aarch64-unknown-linux-gnu -v``` </pre>

- In the "rpi/freq_detect_wasm/freq_detect_pulley_embed/wasmtime-rr-prototyping/target/aarch64-unknown-linux-gnu/debug" directory there should be an "rpi_embed" executable.  SCP this to the RPi Zero 2 using your favorite method (I used winSCP).

- SSH into the RPi using your favorite method (I used mobaXterm), and then run the executable.




