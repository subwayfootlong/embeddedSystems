#ifndef SECRETS_H
#define SECRETS_H

// WiFi Credentials
#define WIFI_SSID "yourmums"
#define WIFI_PASSWORD "deeznuts"

// MQTT Broker settings
#define MQTT_BROKER_IP "192.168.4.1"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "Pico2"

// MQTT Topics
#define TOPIC_CO2 "pico2/sensor/data"  
#define TOPIC_SAFETY_LEVEL "pico4/prediction"
//Each Pico should have its own unique topic to avoid message conflicts

#endif