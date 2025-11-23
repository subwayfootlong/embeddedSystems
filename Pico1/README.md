# **Pico1 – Multi-Gas Monitoring Node (MQ2 + MQ7 + MQ135)**

## **Overview**

Pico1 is a dedicated multi-gas monitoring node within the sensor network, built using the Raspberry Pi Pico W. It interfaces with three distinct analog gas sensors (MQ-2, MQ-7, MQ-135) to detect a wide range of combustible and toxic gases.

The node filters raw sensor data using **Exponential Moving Average (EMA)** and **Oversampling** techniques, then publishes PPM values to an MQTT broker. Pico1 implements **Adaptive Sampling**: it subscribes to a prediction topic (published by Pico 4) to dynamically adjust its sampling rate based on the predicted safety level.

---

## **Hardware Requirements**

| Component | Description |
| :--- | :--- |
| Raspberry Pi Pico W | RP2040 + WiFi Core |
| MQ-2 Sensor | LPG detection |
| MQ-7 Sensor | Carbon Monoxide (CO) detection |
| MQ-135 Sensor | Ammonia (NH3) detection |
| USB cable | Power + serial monitoring |
| Grove 4-pin cable | ADC Connection |

### **Wiring Configuration**

*Note: The assignments below use the standard Pico ADC pins. Please verify `#define` values in your header files match your physical wiring.*

| Sensor | Pico Pin | ADC Channel | Driver File |
| :--- | :--- | :--- | :--- |
| **MQ-2** (LPG) | GP26 | ADC 0 | `mq2_driver.h` |
| **MQ-7** (CO) | GP9 | ADC 1 | `mq7_driver.h` |
| **MQ-135** (NH3) | GP7 | ADC 2 | `mq135_driver.h` |

> **ELECTRICAL WARNING:** The MQ sensors output **0–5V** analog signals. The Pico W ADC pins have a maximum limit of **3.3V**. Connecting the sensor output directly to the Pico **will damage the GPIO**. Use a voltage divider (e.g., 20kΩ/10kΩ) or a logic level shifter.

---

## **Software Stack**

| Component | Purpose |
| :--- | :--- |
| Pico SDK 2.x | Core firmware |
| CYW43 WiFi | WiFi connectivity |
| lwIP | MQTT/WiFi networking |
| Custom MQTT Driver | Publishes/subscribes to topics |
| MQ Drivers | Sensor data, calibration, and error handling |
| Prediction Driver | Manages prediction levels (`NORMAL`, `WARNING`, `HIGH`) |

---

## **Features**

### **1. Multi-Sensor Gas Detection**
Pico1 calculates gas concentrations based on specific sensitivity curves found in the datasheets:

* **MQ-2:** Calibrated for LPG.
* **MQ-7:** Calibrated for Carbon Monoxide (CO).
* **MQ-135:** Calibrated for Ammonia (NH3).

### **2. Adaptive Sampling (Prediction Integration)**
The node adjusts its behavior based on the safety prediction received from **Pico 4**:

| Prediction Level | Sampling Interval |
| :--- | :--- |
| **NORMAL** | 30 Seconds |
| **WARNING** | 10 Seconds |
| **HIGH** | 5 Seconds |

### **3. Noise Filtering**
To ensure stable readings, the drivers employ filtering techniques:
* **MQ-2 & MQ-7:** Uses an **Exponential Moving Average (EMA)** to average out ADC noise.
* **MQ-135:** Uses **Oversampling** (16 reads per sample) to average out ADC noise.

### **4. Error Handling**
* **Warm-up Blocks:** `mq2_ready()` prevents reading the sensor before the heater is stable (approx. 20s).
* **Rail Detection:** The drivers return `EHW` (Hardware Error) if the ADC reads close to 0V or Max Vref, indicating a disconnected wire or short circuit.

---

## **Project Structure**

```
Pico1/
├── main.c              # Main application loop
├── mq2_driver.c        # MQ-2 implementation (LPG)
├── mq2_driver.h
├── mq7_driver.c        # MQ-7 implementation (CO)
├── mq7_driver.h
├── mq135_driver.c      # MQ-135 implementation (NH3)
├── mq135_driver.h
├── pred_driver.c       # Prediction level management
├── pred_driver.h
├── mqtt_driver.c       # MQTT wrapper (connect/publish/subscribe)
├── mqtt_driver.h
├── wifi_driver.c       # WiFi connect/deinit logic
├── wifi_driver.h
├── secrets.h           # WiFi + MQTT credentials
└── CMakeLists.txt      # Build configuration
```

---