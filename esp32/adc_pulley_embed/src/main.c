#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/drivers/uart.h>
#include <stdint.h>

// Forward declaration of your Rust function
extern void rust_main(void);


int main(void) {
    // trace_uart_init();
    printk("Zephyr booting... jumping to Rust\n");

    // Call your Rust entry point
    rust_main();

    // printk("Finished Running\n");

    return 0;
}