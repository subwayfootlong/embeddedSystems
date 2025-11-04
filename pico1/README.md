# Pico W MQTT Project

A complete MQTT client implementation for the Raspberry Pi Pico W that connects to WiFi and communicates with an MQTT broker.

## Project Structure

- **main.c** - Main application entry point with setup calls and main loop
- **wifi_driver.c/h** - WiFi initialization and connection management
- **mqtt_driver.c/h** - MQTT client implementation and message handling
- **secrets.h** - Configuration file for WiFi and MQTT credentials
- **lwipopts.h** - lwIP networking stack configuration
- **CMakeLists.txt** - Build configuration

## Setup Instructions

### 1. Configure Secrets

Edit `secrets.h` and update with your WiFi and MQTT broker details:

```c
// WiFi Credentials
#define WIFI_SSID "Your_WiFi_Network"
#define WIFI_PASSWORD "Your_WiFi_Password"

// MQTT Broker settings
#define MQTT_BROKER_IP "192.168.1.100"  // Your MQTT broker IP
#define MQTT_BROKER_PORT 1883            // Default MQTT port

// MQTT Client ID (unique identifier for this device)
#define MQTT_CLIENT_ID "PicoServer"

// MQTT Topics for testing
#define TOPIC_PUBLISH "test/publish"
#define TOPIC_SUBSCRIBE "test/subscribe"
```

### 2. Build and Upload

```bash
cd build
cmake ..
make
```

Upload the generated `.uf2` file to your Pico W in bootloader mode.

## Usage

### Main Loop - Publishing

Add what you want to publish in this while true loop in main.c

```c
//Example Posting Hello world every 5 seconds
while (true) {
    mqtt_publish_message(TOPIC_PUBLISH, "hello world", 0, 0);
    sleep_ms(5000);
}
```

**Parameters for `mqtt_publish_message()`:**
- `topic` - MQTT topic to publish to
- `payload` - Message content (string)
- `qos` - Quality of Service: `0` (at most once), `1` (at least once), `2` (exactly once)
- `retain` - Retain flag: `0` (don't retain), `1` (retain on broker)

### Publishing Custom Messages

```c
// Publish with different content
mqtt_publish_message("sensor/temperature", "23.5", 1, 0);

// Publish with retain flag
mqtt_publish_message("device/status", "online", 0, 1);
```

### Subscribing to Topics

To subscribe, add the subscribtion in the main function of main.c

```c
int main(void) {
    // Subscribe to topics
    mqtt_subscribe_topic(TOPIC_SUBSCRIBE, 0);
}
```

### Receiving Messages

Incoming messages are handled by the `mqtt_message_received()` callback in `mqtt_driver.c`. By default, it prints received messages to the console:

```c
void mqtt_message_received(const char* topic, const char* payload, uint16_t payload_len) {
    printf("Message received: %.*s\n", payload_len, payload);
}
```


## Serial Debug Output

Connect to the Pico W via USB serial (115200 baud) to see debug messages:

```
=== Pico W MQTT Test ===

1. Connecting to WiFi...
Wi-Fi init OK
Connected to Wi-Fi: Nad_Ismail_Fast
Pico W IP Address: 192.168.0.150

2. Initializing MQTT...
MQTT client initialized (ID: PicoServer)

3. Connecting to MQTT broker...
Connecting to MQTT broker 192.168.0.200:1883...
MQTT connected

4. MQTT Connected Successfully!
Subscribing to topic: test/subscribe
```

## Troubleshooting

### WiFi Connection Failed
- Check SSID and password in `secrets.h`
- Verify the Pico W is in range and the network is 2.4GHz (not 5GHz)
- Check WiFi security type is WPA2

### MQTT Connection Timeout
- Verify broker IP address is correct
- Ensure broker is running and accessible on port 1883
- Check firewall rules allow TCP connections on port 1883
- Verify the Pico W has obtained a valid IP address

### Build Errors
- Ensure `PICO_SDK_PATH` environment variable is set correctly
- Update your Pico SDK to version 1.5.1 or later
- Delete `build` folder and reconfigure with CMake

## Key Functions

**WiFi:**
- `setup_wifi()` - Initialize and connect to WiFi
- `wifi_is_connected()` - Check WiFi connection status
- `wifi_deinit()` - Disconnect WiFi

**MQTT:**
- `setup_mqtt()` - Initialize and connect to MQTT broker
- `mqtt_publish_message(topic, payload, qos, retain)` - Publish message
- `mqtt_subscribe_topic(topic, qos)` - Subscribe to topic
- `mqtt_unsubscribe_topic(topic)` - Unsubscribe from topic
- `mqtt_get_status()` - Get current connection status
- `mqtt_disconnect_client()` - Disconnect from broker

## Notes

- The project uses lwIP for networking and the Pico SDK's CYW43 driver for WiFi
- MQTT QoS levels: 0 (fast, no guarantee), 1 (guaranteed delivery), 2 (exactly once)
- Default keep-alive is set to 60 seconds
- Maximum payload buffer is 256 bytes for incoming messages
