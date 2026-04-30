#include <stddef.h>
#include <stdint.h>

#define WASM_IMPORT(name) __attribute__((import_module("zephyr"), import_name(#name)))


WASM_IMPORT(host-adc-init)
extern int host_adc_init();

WASM_IMPORT(host-adc-read)
extern int host_adc_read();

WASM_IMPORT(host-adc-raw-to-millivolts)
extern int host_adc_raw_to_millivolts(int32_t avgmv);

WASM_IMPORT(host-printk)
extern void host_printk(const char *ptr, unsigned int len);

// WASM_IMPORT(host-dev-msleep)
// extern int host_dev_msleep(int ms);

// WASM_IMPORT(host-k-busy-wait)
// extern void host_k_busy_wait(int us);

WASM_IMPORT(host-sin)
extern double host_sin(double x);

WASM_IMPORT(host-cos)
extern double host_cos(double x);

// WASM_IMPORT(host-get-time-ms)
// extern unsigned int host_get_time_ms(void);
