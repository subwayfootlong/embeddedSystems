

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

No other installation steps are needed.

---

