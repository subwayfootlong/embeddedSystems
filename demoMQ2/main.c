#include <stdio.h>
#include "pico/stdlib.h"
#include "mq2_driver.h"

int main()
{
    stdio_init_all();

    // Wait for USB serial connection
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("=== Pico W MQ2 Gas Sensor Demo ===\n");

    // Set MQ2 configuration values
    Mq2Config cfg = {
        .adc_channel     = 0,
        .warmup_ms       = 20000,
        .min_interval_ms = 1000
    };

    // Initialize MQ2
    if (mq2_init(&cfg) != MQ2_OK) {
        printf("Failed to initialize MQ2 driver.\n");
        return -1;
    }

    // Initialize MQ2 warmup
    mq2_warmup();

    uint32_t elapsed_seconds = cfg.warmup_ms / 1000;
    while (!mq2_ready()) {
        printf("Warming up heater, please wait %useconds...\n", elapsed_seconds);
        sleep_ms(1000);
        elapsed_seconds--;
    }

    printf("Warmup complete. Sensor ready.\n");

    while (true) {
        float ppm = 0.0f;
        float voltage = 0.0f;
        int status = mq2_sample(&ppm, &voltage);

        if (status == MQ2_OK) {
            printf("MQ2 reading: %.2fppm, Voltage: %.2fV\n", ppm, voltage);
        } else {
            switch (status) {
                case MQ2_ERR_INVAL:
                    printf("Invalid configuration values.\n");
                    break;
                case MQ2_ERR_BUSY:
                    printf("Sampling too quickly or warmup incomplete.\n");
                    break;
                case MQ2_ERR_HW:
                    printf("Hardware fault detected.\n");
                    break;
                case MQ2_ERR_NO_INIT:
                    printf("MQ2 driver not initialized.\n");
                    break;
                default:
                    printf("Error: %d\n", status);
                    break;
            }
        }

        sleep_ms(cfg.min_interval_ms);
    }

    return 0;
}