#ifndef SECRETS_H
#define SECRETS_H

// WiFi Credentials
#define WIFI_SSID "HomeWifi"
#define WIFI_PASSWORD "T0111494"

// MQTT Broker settings
#define MQTT_BROKER_IP "192.168.68.125"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "PicoTest"

// MQTT Topics
#define TOPIC_PUBLISH "test/publish"        
#define TOPIC_SUBSCRIBE "test/subscribe"
//Each Pico should have its own unique topic to avoid message conflicts

#endif