Pico4 — ML Inference Example (TensorFlow Lite Micro)

Project overview
----------------
Pico4 is a TensorFlow Lite Micro (TFLM) inference for the Raspberry Pi Pico (RP2040 / Pico W). It demonstrates embedding a compiled TFLite model in firmware (`model_data.cc`), running inference with the TFLM interpreter (`ml_inference.*`), and integrating inference outputs with the project's networking stack (Wi‑Fi and MQTT).

What this project demonstrates 
-----------------------------------------
- Model embedding: the model is included as a C array in `model_data.cc` and statically linked into the firmware.
- Inference pipeline: `ml_inference.cpp` prepares inputs, configures the TFLM interpreter, runs inference, and provides a simple API for the application.
- System integration: `main.c` shows how sensor/preprocessed inputs are collected, fed into the model, and results are published over MQTT using `mqtt_driver`.

Files and responsibilities
--------------------------
- `CMakeLists.txt` — Project build rules for the Pico4 target. Sets compiler flags, lists source files, and (optionally) adds the `pico-tflmicro` subdirectory.
- `main.c` — Program entry point and runtime flow: initialization, periodic inference calls, and publishing of results.
- `ml_inference.cpp` / `ml_inference.h` — Inference module that:
  - configures and allocates the TFLM interpreter,
  - copies and preprocesses inputs into the interpreter tensors,
  - invokes inference and postprocesses outputs,
  - exposes a small API used by `main.c`.
- `model_data.cc` — Generated C source containing the model bytes (the `.tflite` payload). Replace this file to change the on-device model.
- `mqtt_driver.c` / `mqtt_driver.h` — MQTT publish/subscribe helpers used to report inference results.
- `wifi_driver.c` / `wifi_driver.h` — Wi‑Fi connection helpers (Pico W) used to bring up the network interface.
- `lwipopts.h` — lwIP configuration header to tune stack options required by MQTT and the application.
- `secrets.h` — Credentials and network configuration for Wi‑Fi and MQTT (SSID, password, broker address, etc.). Treat this file as local-only and do not commit real credentials to public VCS.
- `mltrainer.py` - This script performs a complete supervised machine-learning pipeline: it loads your labelled gas-sensor dataset, scales the input features, trains a Multilayer Perceptron (feed-forward neural network) for 3-class safety classification using ReLU hidden layers, softmax output, Adam optimizer, and cross-entropy loss, evaluates the model with accuracy and confusion matrix, and finally converts the trained model into a quantized TensorFlow Lite file and C array for deployment on microcontrollers.

Important notes
-----------------------------
- Focus on `ml_inference.*`, `model_data.cc`, and `main.c` to evaluate ML logic, input mapping, and system behavior.
- The networking code (`wifi_driver`, `mqtt_driver`) is auxiliary and demonstrates how results are exported; inference still runs even without network connectivity.
- `secrets.h` is intentionally local-only; if absent, the firmware will skip network tests but inference functionality itself still runs.


# Appendix 

# Installing TensorFlow Lite Micro (pico-tflmicro)

This project uses **pico-tflmicro** as an external library for TensorFlow Lite Micro inference on Raspberry Pi Pico / Pico W.

Follow these steps to install it.

---

## 1. Clone pico-tflmicro (must be at the same level as Pico4 folder)

From your **project root directory**, run:

```sh
git clone https://github.com/raspberrypi/pico-tflmicro.git
```

Your folder structure should now look like:

```
/project-root
    /Pico4
    /pico-tflmicro
```

---

## 2. Initialize submodules

Inside the cloned library:

```sh
cd pico-tflmicro
git submodule update --init --recursive
```

This downloads the TensorFlow Lite Micro source files required for building on RP2040.

---

## 3. Build normally with your Pico4 project

Your Pico4 CMakeLists should already reference:

```
add_subdirectory(../pico-tflmicro pico-tflmicro)
target_link_libraries(Pico4 pico-tflmicro)
```



