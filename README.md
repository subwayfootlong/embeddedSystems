# embeddedSystems

This repository contains several Raspberry Pi Pico / embedded projects, examples, drivers, and tooling used for experiments and demos. Below is a concise description of each top-level folder and key files so you can quickly find what you need.

**Project Structure**
- **`CMakeLists.txt`**: Root CMake file for the workspace — entry point for building subprojects.
- **`builds/`**: Prebuilt `.uf2` images for quick flashing of supported boards (e.g. `Pico1.uf2`).
- **`Pico1/`**: Gas-sensor (MQ2, MQ7, MQ135) that publishes to MQTT with power management
- **`Pico2/`**: ADC1011 sensor that publishes to MQTT with power management
- **`Pico3/`**: Data-aggregator that logs sensor data to SD and serves a HTTP dashboard (`index.html`). 
- **`Pico4/`**: Machine learning  using TensorFlow Lite Micro. Provides classification inference based on sensors

See each subfolder's `README.md` or `CMakeLists.txt` for project-specific build and configuration details.

**Required Libraries**
- **`nofatos`**: (No-OS FatFS / SD over SPI) — Required for the SD card and FatFS examples under `no-OS-FatFS-SD-SPI-RPi-Pico`. Some examples include their own drivers, but larger demos expect a no-OS FatFS/SPI driver to be available to the build system. If your build fails with missing FatFS or SD driver symbols, ensure the no-OS FatFS/SPI code is available to CMake or placed under the project's `FatFs_SPI` folder.
- **`tflmicro`**: TensorFlow Lite for Microcontrollers — Required for the ML inference example in `Pico4/` and the `tflmicro/` helper scripts. You can fetch the upstream TFLite Micro sources and point your build at them. Example (powershell):

```powershell
cd tflmicro
git clone https://github.com/raspberrypi/pico-tflmicro.git pico-tflmicro
```

After cloning, follow the `tflmicro/README.md` and your project's `CMakeLists.txt` to ensure CMake finds the `pico-tflmicro` sources. Also ensure the Raspberry Pi Pico SDK/toolchain is installed and available to CMake.

**Clone Commands (examples)**
- **`tflmicro` (Pico TFLite Micro port)** — clone into the `tflmicro` folder:

```powershell
cd c:\Users\admin\Documents\GitHub\embeddedSystems\tflmicro
git clone https://github.com/raspberrypi/pico-tflmicro.git pico-tflmicro
```

- **`nofatos` (No-OS FatFS / SD driver)** — replace `<NOFATOS_REPO_URL>` with the repository you use for the no-OS FatFS/SPI driver. A common layout is to place it under the project's `FatFs_SPI` folder so CMake can find it:

```powershell
cd c:\Users\admin\Documents\GitHub\embeddedSystems\no-OS-FatFS-SD-SPI-RPi-Pico\FatFs_SPI
git clone <NOFATOS_REPO_URL> nofatos
```

**Required folder structure** (after cloning the two libraries)

```
embeddedSystems/
	CMakeLists.txt
	builds/
	no-OS-FatFS-SD-SPI-RPi-Pico/
	tflmicro/
	Pico1/
	Pico2/
	Pico3/
	Pico4/
	tests/
```

Place the cloned folders where your CMakeLists expect them (or update CMake variables to point to the cloned locations).

**How to build a Pico project (general)**
1. Install the Pico SDK and toolchain following Raspberry Pi Pico documentation.
2. From the workspace or a specific project folder (for example `Pico1`):

```powershell
cd Pico1
mkdir build; cd build
cmake ..
cmake --build .
```

3. If building from the repo root, pass the correct project path to CMake or open the appropriate subfolder in your IDE. Some subprojects include `pico_sdk_import.cmake` to help find the Pico SDK.

4. Copy the generated `.uf2` to your Pico device (or use `builds/` for prebuilt UF2 files).


**Notes & Tips**
- Many subprojects rely on a `secrets.h` file (contains Wi-Fi credentials and MQTT settings). Do not commit sensitive credentials to public repos.
- Examples often include board-specific hardware config files like `hw_config.c` or `thing_plus.hw_config.c` — check those for pin mappings.
- `tflmicro/` contains helper scripts to prepare models for on-device inference; see its `README.md` for model conversion and example usage.

---
Generated on repository snapshot (concise summary). For detailed build instructions, open the specific subfolder's `README.md` or `CMakeLists.txt`.

