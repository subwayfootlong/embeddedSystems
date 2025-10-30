#ifndef SECRETS_H
#define SECRETS_H

// WiFi Credentials
#define WIFI_SSID "Nad_Ismail_Fast"
#define WIFI_PASSWORD "devices123"

// MQTT Broker settings
#define MQTT_BROKER_IP "192.168.0.200"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "PicoServer"

// MQTT Topics for Timestamp synchronization topics
#define TOPIC_TIMESTAMP_REQUEST "pc/timestamp/request"
#define TOPIC_TIMESTAMP_REPLY "pc/timestamp/reply"

// MQTT Topics for picos
#define TOPIC_PICO1 "pico1/sensor/data"
#define TOPIC_PICO2 "pico2/sensor/data"

#endif