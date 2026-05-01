#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include <stdint.h>
#include <zephyr/drivers/uart.h>
#include <math.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/wifi_mgmt.h>
// #include <zephyr/net/wifi_credentials.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_if.h>



static const struct adc_dt_spec adc_chan =
	ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

static const struct device *rr_uart = 
    DEVICE_DT_GET(DT_CHOSEN(rr_uart));

int16_t sample;

struct adc_sequence sequence = {
    .buffer = &sample,
    .buffer_size = sizeof(sample),
};

void host_rr_write(const uint8_t *ptr, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uart_poll_out(rr_uart, ptr[i]);
    }
}


int host_rr_uart_init(void) {
    if (!device_is_ready(rr_uart)) {
        printk("RR UART not ready\n");
        return -19;
    }
    printk("RR UART ready and modified!\n");
    // const char *test = "RR UART OK\r\n";
    // for (int i = 0; test[i]; i++) {
    //     uart_poll_out(rr_uart, test[i]);
    // }
    return 0;
}

void host_printk(const char *ptr, size_t len){
    printk("%.*s", (int)len, ptr);
    return;
}

int host_adc_init(){
    printk("host_adc_init entered!\n");
    int err;
    err = adc_channel_setup_dt(&adc_chan);
	if (err) {
		printk("adc_channel_setup_dt failed: %d\n", err);
		return err;
	}

	err = adc_sequence_init_dt(&adc_chan, &sequence);
	if (err) {
		printk("adc_sequence_init_dt failed: %d\n", err);
		return err;
	}

    host_rr_uart_init();

    return 0;
}

int host_adc_read(){
    int err;
    err = adc_read_dt(&adc_chan, &sequence);
    if (err) {
        printk("adc_read_dt failed: %d\n", err);
        return err;
    }
    return (int)sample;
}

int host_adc_raw_to_millivolts(int32_t avg_mv){
    adc_raw_to_millivolts_dt(&adc_chan, &avg_mv);
    return avg_mv;
}

// void host_k_busy_wait(int us) {
//     k_busy_wait(us);
//     return;
// }

// int host_dev_msleep(int ms) {
//     int32_t conv_ms = (int32_t)ms;
//     int32_t conv_ret = (int32_t)k_msleep(conv_ms);
//     return conv_ret;
// }


double host_sin(double x) { return sin(x); }
double host_cos(double x) { return cos(x); }
// uint32_t host_get_time_ms(void) { return (uint32_t)k_uptime_get(); }



