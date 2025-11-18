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
// MQTT callback - FIXED VERSION
// -----------------------------------------------------------------------------
void mqtt_message_callback(const char* topic, const char* payload, uint16_t payload_len) {
    if (!topic || !payload) return;
    
    printf("[MQTT] Topic: '%s', Payload: %.*s\n", topic, payload_len, payload);

    // pico1: "LPG,CO,NH3"
    if (strcmp(topic, "pico1/sensor/data") == 0) {
        float lpg, co, nh3;
        if (sscanf(payload, "%f,%f,%f", &lpg, &co, &nh3) == 3) {
            g_LPG = lpg;
            g_CO = co;
            g_NH3 = nh3;
            g_has_pico1 = true;
            printf("[DATA] pico1 update: LPG=%.2f CO=%.2f NH3=%.2f\n", lpg, co, nh3);
        } else {
            printf("[ERROR] Failed to parse pico1 data: %.*s\n", payload_len, payload);
        }
    }
    // pico2: "CO2"
    else if (strcmp(topic, "pico2/sensor/data") == 0) {
        float co2;
        if (sscanf(payload, "%f", &co2) == 1) {
            g_CO2 = co2;
            g_has_pico2 = true;
            printf("[DATA] pico2 update: CO2=%.2f\n", co2);
        } else {
            printf("[ERROR] Failed to parse pico2 data: %.*s\n", payload_len, payload);
        }
    }
    else {
        printf("[WARNING] Unknown topic: %s\n", topic);
    }
}

// -----------------------------------------------------------------------------
// Print classification result
// -----------------------------------------------------------------------------
static void print_classification(int cls) {
    const char* levels[] = {"NORMAL", "WARNING", "HIGH"};
    if (cls >= 0 && cls <= 2) {
        printf("[ML] Prediction: %s\n", levels[cls]);
        
        // Publish prediction to MQTT
        char prediction_msg[32];
        snprintf(prediction_msg, sizeof(prediction_msg), "%s", levels[cls]);
        mqtt_publish_message(TOPIC_PREDICTION, prediction_msg, 0, 0);
        printf("[MQTT] Published prediction: %s\n", levels[cls]);
    } else {
        printf("[ML] ERROR (code=%d)\n", cls);
    }
}

// -----------------------------------------------------------------------------
// Test ML inference
// -----------------------------------------------------------------------------
static void test_ml_inference(void) {
    printf("\n[TEST] Running test inference...\n");
    int prediction = ml_inference_run(195.0f, 283.0f, 212.0f, 113.0f);
    print_classification(prediction);
}

// -----------------------------------------------------------------------------
// main()
// -----------------------------------------------------------------------------
int main() {
    stdio_init_all();
    sleep_ms(2000);  // Wait for serial connection
    
    printf("\n=== Pico4 ML Inference Node ===\n");

    // -------------------------------------------------------------------------
    // 1. Initialize Wi-Fi
    // -------------------------------------------------------------------------
    printf("1. Initializing WiFi...\n");
    if (wifi_init() != WIFI_OK) {
        printf("ERROR: WiFi init failed\n");
        return 1;
    }
    
    if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, 30000) != WIFI_OK) {
        printf("ERROR: WiFi connect failed\n");
        return 1;
    }
    printf("WiFi Connected! IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));

    // -------------------------------------------------------------------------
    // 2. Initialize MQTT
    // -------------------------------------------------------------------------
    printf("2. Initializing MQTT...\n");
    if (mqtt_init(MQTT_CLIENT_ID) != MQTT_OK) {
        printf("ERROR: MQTT init failed\n");
        return 1;
    }
    
    // Connect to MQTT broker
    if (mqtt_connect(MQTT_BROKER_IP, MQTT_BROKER_PORT, mqtt_message_callback) != MQTT_OK) {
        printf("ERROR: MQTT connect failed\n");
        return 1;
    }
    
    // Wait for connection with timeout
    printf("Waiting for MQTT connection...\n");
    int timeout = 20;
    while (mqtt_get_status() != MQTT_STATUS_CONNECTED && timeout > 0) {
        sleep_ms(1000);
        timeout--;
        printf(".");
    }
    printf("\n");
    
    if (mqtt_get_status() != MQTT_STATUS_CONNECTED) {
        printf("ERROR: MQTT connection timeout\n");
        return 1;
    }
    printf("MQTT Connected!\n");

    // Subscribe to topics
    if (mqtt_subscribe_topic(TOPIC_PICO1, 0) != MQTT_OK) {
        printf("WARNING: Failed to subscribe to %s\n", TOPIC_PICO1);
    }
    
    if (mqtt_subscribe_topic(TOPIC_PICO2, 0) != MQTT_OK) {
        printf("WARNING: Failed to subscribe to %s\n", TOPIC_PICO2);
    }
    
    printf("Subscribed to topics:\n- %s\n- %s\n", TOPIC_PICO1, TOPIC_PICO2);

    // -------------------------------------------------------------------------
    // 3. Initialize ML Inference
    // -------------------------------------------------------------------------
    printf("3. Initializing ML Inference...\n");
    if (!ml_inference_init()) {
        printf("ERROR: ML inference init failed\n");
        return 1;
    }
    printf("ML Inference Ready!\n");

    // Test ML inference (optional)
    // test_ml_inference();

    // -------------------------------------------------------------------------
    // Main Loop
    // -------------------------------------------------------------------------
    printf("\n=== Starting Main Loop ===\n");
    printf("Waiting for sensor data from pico1 and pico2...\n");
    
    uint32_t last_status_print = 0;
    
    while (true) {
        // Handle network background tasks
        cyw43_arch_poll();
        mqtt_poll();

        // Print status every 10 seconds
        if (to_ms_since_boot(get_absolute_time()) - last_status_print > 10000) {
            printf("[STATUS] MQTT: Connected, Data: pico1=%d pico2=%d\n",
                   g_has_pico1, g_has_pico2);
            last_status_print = to_ms_since_boot(get_absolute_time());
        }

        // Run inference when both sensors have new data
        if (g_has_pico1 && g_has_pico2) {
            printf("\n[INFERENCE] Running ML on: LPG=%.2f, CO=%.2f, NH3=%.2f, CO2=%.2f\n",
                   g_LPG, g_CO, g_NH3, g_CO2);
            
            int prediction = ml_inference_run(g_LPG, g_CO, g_NH3, g_CO2);
            print_classification(prediction);
            
            // Reset flags for next inference
            g_has_pico1 = false;
            g_has_pico2 = false;
            
            printf("Waiting for new sensor data...\n");
        }

        sleep_ms(100);
    }

    return 0;
}