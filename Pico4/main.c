#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "wifi_driver.h"
#include "mqtt_driver.h"
#include "ml_inference.h"
#include "secrets.h"

// -----------------------------------------------------------------------------
// Global storage for sensor values
// -----------------------------------------------------------------------------
static float g_LPG  = 0.0f;
static float g_CO   = 0.0f;
static float g_NH3  = 0.0f;
static float g_CO2  = 0.0f;

static bool g_has_pico1 = false;
static bool g_has_pico2 = false;

// -----------------------------------------------------------------------------
// MQTT callback – called when any subscribed topic receives a message
// -----------------------------------------------------------------------------
void mqtt_message_callback(const char *topic, const char *payload)
{
    if (!topic || !payload) return;

    // pico1: "LPG,CO,NH3"
    if (strcmp(topic, "pico1/sensor/data") == 0) {
        float lpg, co, nh3;

        if (sscanf(payload, "%f,%f,%f", &lpg, &co, &nh3) == 3) {
            g_LPG = lpg;
            g_CO = co;
            g_NH3 = nh3;
            g_has_pico1 = true;
            printf("[MQTT] pico1 update: LPG=%.2f CO=%.2f NH3=%.2f\n", lpg, co, nh3);
        }
    }

    // pico2: "CO2"
    else if (strcmp(topic, "pico2/sensor/data") == 0) {
        float co2;

        if (sscanf(payload, "%f", &co2) == 1) {
            g_CO2 = co2;
            g_has_pico2 = true;
            printf("[MQTT] pico2 update: CO2=%.2f\n", co2);
        }
    }
}

// -----------------------------------------------------------------------------
// Print classification result
// -----------------------------------------------------------------------------
static void print_classification(int cls)
{
    if (cls == 0)      printf("[ML] Prediction: NORMAL\n");
    else if (cls == 1) printf("[ML] Prediction: MEDIUM\n");
    else if (cls == 2) printf("[ML] Prediction: HIGH\n");
    else               printf("[ML] ERROR\n");
}

// -----------------------------------------------------------------------------
// main()
// -----------------------------------------------------------------------------
int main()
{
    stdio_init_all();
    sleep_ms(2000);

    printf("\n=== Pico4 ML Inference Node ===\n");

    // -----------------------------------------------------------------------------
    // 1. Init Wi-Fi
    // -----------------------------------------------------------------------------
    if (!wifi_init()) {
        printf("WiFi init failed\n");
        return 1;
    }

    printf("WiFi OK\n");

    // -----------------------------------------------------------------------------
    // 2. Init MQTT
    // -----------------------------------------------------------------------------
    mqtt_set_message_callback(mqtt_message_callback);

    if (!mqtt_init()) {
        printf("MQTT init failed\n");
        return 1;
    }

    printf("MQTT connected\n");

    mqtt_subscribe_topic(TOPIC_PICO1, 0);
    mqtt_subscribe_topic(TOPIC_PICO2, 0);

    // -----------------------------------------------------------------------------
    // 3. Init ML
    // -----------------------------------------------------------------------------
    if (!ml_inference_init()) {
        printf("ML init failed\n");
        return 1;
    }

    printf("ML initialized\n");

    // -----------------------------------------------------------------------------
    // Main loop
    // -----------------------------------------------------------------------------
    while (true) {

        // Needed for lwIP background processing
        cyw43_arch_poll();
        mqtt_poll();

        // We run inference only when BOTH picos have sent new data
        if (g_has_pico1 && g_has_pico2) {

            printf("\n[DATA] Running inference on: LPG=%.2f CO=%.2f NH3=%.2f CO2=%.2f\n",
                g_LPG, g_CO, g_NH3, g_CO2);

            int cls = ml_inference_run(g_LPG, g_CO, g_NH3, g_CO2);

            print_classification(cls);

            // reset flags so we only infer on next update
            g_has_pico1 = false;
            g_has_pico2 = false;
        }

        sleep_ms(10);
    }

    return 0;
}
