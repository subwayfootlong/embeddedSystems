#ifndef SECRETS_H
#define SECRETS_H

// WiFi Credentials
#define WIFI_SSID "lutfiesp8266"
#define WIFI_PASSWORD "ilovesit"

// MQTT Broker settings
#define MQTT_BROKER_IP "192.168.4.10"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "Pico1"

// MQTT Topics
#define TOPIC_CO2 "pico1/sensor/data"        
#define TOPIC_SAFETY_LEVEL "pico3/safety_level"
//Each Pico should have its own unique topic to avoid message conflicts

#endif