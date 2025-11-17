#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID "yourmums"
#define WIFI_PASSWORD "deeznuts"

// MQTT Broker settings
#define MQTT_BROKER_IP "192.168.4.1"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "Pico4"

// MQTT Topics
#define TOPIC_PUBLISH "test/publish"        
#define TOPIC_SUBSCRIBE "test/subscribe"
//Each Pico should have its own unique topic to avoid message conflicts
#define TOPIC_PICO1 "pico1/sensor/data"
#define TOPIC_PICO2 "pico2/sensor/data"
#endif