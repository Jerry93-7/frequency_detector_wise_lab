#include "shim.h"
#include <stdarg.h>
#include <stdint.h>
#include "kiss_fft.h"

#define N 512
// #define M_PI 3.14159265358979323846f
#define SAMPLE_RATE 8000  // Hz
#define SAMPLE_PERIOD_US (1000000 / SAMPLE_RATE)

static int16_t analysis_buf[N];
static kiss_fft_cpx fft_in[N];
static kiss_fft_cpx fft_out[N];
static float hann_window[N];
static char fft_mem[8192];
static kiss_fft_cfg fft_cfg;
static int g_sample_rate;

static void wasm_printk(const char *fmt, ...) {
    static char buf[96];
    int i = 0;
    va_list ap;
    va_start(ap, fmt);
    while (*fmt && i < 94) {
        if (fmt[0] == '%') {
            if (fmt[1] == 's') {
                const char *s = va_arg(ap, const char *);
                while (*s && i < 94) buf[i++] = *s++;
                fmt += 2;
            } else if (fmt[1] == 'd' || (fmt[1] == 'l' && fmt[2] == 'd')) {
                long d = (fmt[1] == 'l') ? va_arg(ap, long) : va_arg(ap, int);
                if (fmt[1] == 'l') fmt += 3; else fmt += 2;
                if (d < 0) { buf[i++] = '-'; d = -d; }
                char tmp[12]; int ti = 0;
                if (d == 0) { tmp[ti++] = '0'; }
                while (d > 0) { tmp[ti++] = '0' + (d % 10); d /= 10; }
                while (ti-- > 0 && i < 94) buf[i++] = tmp[ti];
            } else {
                buf[i++] = *fmt++;
            }
        } else {
            buf[i++] = *fmt++;
        }
    }
    va_end(ap);
    buf[i] = '\0';
    host_printk(buf, (unsigned int)i);
}

static int detect_best_freq_from_i16(const int16_t *samples, int sample_rate) {
    if (!samples || sample_rate <= 0 || !fft_cfg) return 0;

    // DC bias removal
    float mean = 0.0f;
    for (int i = 0; i < N; i++) {
        mean += (float)samples[i];
    }
    mean /= (float)N;

    // Apply Hann window
    for (int i = 0; i < N; i++) {
        fft_in[i].r = ((float)samples[i] - mean) * hann_window[i];
        fft_in[i].i = 0.0f;
    }

    // Run FFT
    kiss_fft(fft_cfg, fft_in, fft_out);

    // Find peak magnitude bin in 20-2000 Hz range
    int min_bin = (int)(20.0f * (float)N / (float)sample_rate);
    int max_bin = (int)(2000.0f * (float)N / (float)sample_rate);
    if (min_bin < 1) min_bin = 1;
    if (max_bin > (N / 2) - 2) max_bin = (N / 2) - 2;

    int peak_bin = -1;
    float peak_mag = 0.0f;
    float avg_mag = 0.0f;
    int count = 0;
    for (int i = min_bin; i <= max_bin; i++) {
        float re = fft_out[i].r;
        float im = fft_out[i].i;
        float mag = re * re + im * im;
        if (mag > peak_mag) {
            peak_mag = mag;
            peak_bin = i;
        }
        avg_mag += mag;
        count++;
    }

    // if (peak_mag < 5.0f * avg_mag) {
    //     return 0;
    // }

    if (peak_bin < 0) return 0;

    // wasm_printk("peak_bin = %d\n", peak_bin);

    return peak_bin * sample_rate / N;
}


int run(void)
{
    host_adc_init();

    // Allocate FFT state into static buffer (no malloc needed)
    size_t fft_len = sizeof(fft_mem);
    fft_cfg = kiss_fft_alloc(N, 0, fft_mem, &fft_len);
    if (!fft_cfg) {
        wasm_printk("fft alloc failed\n");
        return 1;
    }

    // Precompute Hann window
    for (int i = 0; i < N; i++) {
        hann_window[i] = 0.5f * (1.0f - __builtin_cosf(2.0f * M_PI * (float)i / (float)(N - 1)));
    }

    // Measure sample rate
    // unsigned int t0 = host_get_time_ms();
    // for (int i = 0; i < N; i++) {
    //     analysis_buf[i] = (int16_t)host_adc_read();
    // }
    // unsigned int t1 = host_get_time_ms();
    // unsigned int elapsed = t1 - t0;
    // if (elapsed == 0) elapsed = 1;
    // g_sample_rate = (N * 1000) / elapsed;
    // wasm_printk("sample_rate=%d\n", g_sample_rate);
    
	unsigned int count = 0;
	unsigned int total_time = 0;

    while (1) {
        // unsigned int loop0 = host_get_time_ms();
        // unsigned int t0 = host_get_time_ms();
        for (int i = 0; i < N; i++) {
            analysis_buf[i] = (int16_t)host_adc_read();
            // host_k_busy_wait(0);
        }
        // unsigned int t1 = host_get_time_ms();
        // unsigned int elapsed = t1 - t0;
        // if (elapsed == 0) elapsed = 1;
        // g_sample_rate = (N * 1000) / elapsed;
        // uint32_t fs = (N * 1000U) / elapsed;
        wasm_printk("sample_rate=%d\n", g_sample_rate);
        // g_sample_rate = 2560;
        // wasm_printk("x0=%d x1=%d x2=%d x3=%d\n", analysis_buf[0], analysis_buf[1], analysis_buf[2], analysis_buf[3]);
        int freq = detect_best_freq_from_i16(analysis_buf, 2560);
        if (freq > 0) {
            wasm_printk("freq=%d Hz\n", freq);
        } else {
            wasm_printk("no signal\n");
        }
        // unsigned int loop1 = host_get_time_ms();
        // unsigned int loop_elapsed = loop1 - loop0;
        // // if (loop_elapsed == 0) loop_elapsed = 1;
        // // unsigned int loop_time = (N * 1000) / loop_elapsed;
        // total_time += loop_elapsed;
        // count++;
        // wasm_printk("sample %d: total_time=%d\n", count, total_time);
    }

    return 0;
}
