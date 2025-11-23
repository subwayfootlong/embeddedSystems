# **Pico2 – CO₂ Monitoring Node (ACD1100 + MQTT + EMA Filter)**

## **Overview**

Pico2 is a low-power CO₂ monitoring node built using the Raspberry Pi Pico W and the ASAIR ACD1100 CO₂ sensor.
The device periodically measures CO₂ concentration, applies an EMA filter for smoothing, and publishes the values to an MQTT broker.
It also listens for safety-level updates from the cloud to dynamically adjust its sampling interval.

The node supports:

* WiFi connectivity using the CYW43 driver
* MQTT publish/subscribe
* EMA filtering (configurable α)
* Power-saving mode (wifi off + low clock)
* Error handling for ACD1100 (I2C, CRC, invalid readings)

---

## **Hardware Requirements**

| Component                   | Description               |
| --------------------------- | ------------------------- |
| Raspberry Pi Pico W         | RP2040 + WiFi             |
| ACD1100 / ACD10 CO₂ sensor  | I2C interface, 5V supply  |
| USB cable                   | Power + serial monitoring |
| Optional: Keweisi USB meter | Power debugging           |

### **Wiring (I2C1)**

| ACD1100 Pin | Pico Pin        |
| ----------- | --------------- |
| VCC (5V)    | VBUS            |
| GND         | GND             |
| SDA         | GP26 (I2C1 SDA) |
| SCL         | GP27 (I2C1 SCL) |

Pull-ups must be 3.3V (use level shifter if your breakout uses 5V pull-ups).

---

## **Software Stack**

| Component          | Purpose                        |
| ------------------ | ------------------------------ |
| Pico SDK 2.x       | Core firmware                  |
| CYW43 WiFi stack   | WiFi connectivity              |
| lwIP               | MQTT/WiFi networking           |
| Custom MQTT driver | Publishes/subscribes to topics |
| ACD1100 driver     | Validates CRC, parses frames   |
| EMA filter         | Smooths CO₂ output             |
| Power manager      | Low-power mode handling        |

---

## **Features**

### **CO₂ Measurement**

* Reads 9-byte ACD1100 data frame
* Validates CRC-8 (0x31 polynomial, init 0xFF)
* 32-bit PPM value extraction
* Raw temperature output available

### **Error Handling**

The driver returns structured error codes:

```c
typedef enum {
    ACD1100_OK = 0,
    ACD1100_ERR_I2C   = -1,
    ACD1100_ERR_FORMAT = -2,
    ACD1100_ERR_CRC    = -3,
    ACD1100_ERR_RANGE  = -4,
    ACD1100_ERR_INVAL  = -5
} acd1100_status_t;
```

Errors are printed to USB serial for debugging.

### **EMA Filtering**

Smooths sensor noise:

```
EMA = α * new_value + (1 – α) * previous
```

Configured in firmware:

```c
ema_init(0.25f);
```

### **MQTT Cloud Integration**

* Publishes filtered CO₂ PPM → `topic/co2`
* Subscribes to safety level → adjusts sampling rate
* Reconnect logic to prevent hangs

### **Dynamic Power Modes**

* **Normal mode** → sleeps (5–10 minutes) with WiFi OFF
* **Warning / High** → stays awake, samples more frequently
* CPU clock scaling supported (48 MHz low-power mode)

---

## **Project Structure**

```
Pico2/
 ├── acd1100.c            # CO₂ sensor driver + error handling
 ├── acd1100.h
 ├── ema_filter.c         # EMA filter implementation
 ├── ema_filter.h
 ├── mqtt_driver.c        # MQTT wrapper (connect/publish/subscribe)
 ├── mqtt_driver.h
 ├── wifi_driver.c        # WiFi connect/deinit logic
 ├── wifi_driver.h
 ├── power_manager.c      # Low-power mode control
 ├── power_manager.h
 ├── main.c               # Main application loop
 └── secrets.h            # WiFi + MQTT credentials
```

---

## **Build Instructions**

### **1. Install Pico SDK**

Follow Raspberry Pi’s official documentation or use `pico-setup.sh`.

### **2. Configure Build**

```
mkdir build
cd build
cmake ..
```

### **3. Compile**

```
make -j4
```

### **4. Flash to Pico W**

Hold BOOTSEL → connect USB → copy `.uf2` file.

---

## **Running the Node**

Open Serial Monitor:

```
tio /dev/tty.usbmodemXXXX -b 115200
```

Expected startup sequence:

```
1. Connecting to WiFi...
2. Initializing MQTT...
3. Subscribing: pico4/prediction
4. Sensor starting...
CO2: raw=550 ppm, filtered=545 ppm
```

If an error occurs:

```
[ACD1100 ERROR] CRC mismatch (calc=0xF2 recv=0x7A)
```

---

## **Testing & Validation**

### **Sensor Simulation Tests**

| Test                    | Expected Result      |
| ----------------------- | -------------------- |
| Unplug sensor           | `ACD1100_ERR_I2C`    |
| Force invalid header    | `ACD1100_ERR_FORMAT` |
| Modify CRC byte         | `ACD1100_ERR_CRC`    |
| Cover vent to raise CO₂ | Increased PPM values |
| Apply EMA α=0.1         | Heavy smoothing      |
| Apply EMA α=0.8         | Fast response        |

### **MQTT Tests**

* Verify messages on: `mosquitto_sub -t topic/co2`
* Send safety-level commands to topic → pico adjusts interval

### **Power Mode Tests**

* Measure USB current using Keweisi meter
* WiFi disconnect → current should drop by ~30–50 mA

---

## **Known Issues**

* ACD1100 warm-up may take several seconds
* Rare `tcp_write: no pbufs` indicates lwIP memory pressure
  → fixed by reducing publish frequency

---

## **Future Improvements**

* Add temperature compensation
* Add NVS storage for last EMA value
* Implement offline caching when MQTT unavailable
* Support OTA firmware update

---
