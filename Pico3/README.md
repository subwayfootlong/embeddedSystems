# Pico3 — Data Aggregator & SD Logging (no-OS FatFS / SD over SPI)

Project overview
----------------
Pico3 is a data-aggregator firmware for Raspberry Pi Pico devices that collects sensor data, logs it to an SD card using FatFS over SPI (the `nofatos` no-OS driver), and serves a small HTTP dashboard to view recent logs. The project demonstrates robust SD card integration, simple web serving, and optional networked telemetry via MQTT/Wi‑Fi.

What this project demonstrates 
-----------------------------------------
- SD card integration: mounting, reading, writing, and rotating log files using FatFS and the SPI SD driver.
- Filesystem robustness: handling SD card insertion/removal, write failures, and graceful recovery.
- HTTP static file serving: `http_server_driver` exposes a small dashboard (`index.html`) and can stream or download log files.
- System-level integration: coordination between `main.c`, `sd_driver`, `http_server_driver`, and networking components.

Files and responsibilities
--------------------------
- `CMakeLists.txt` — Build rules for the Pico3 target. Adds sources and links to the no-OS FatFS/SD driver when present.
- `main.c` — Application entry point. Initializes peripherals, mounts the SD card, schedules sensor reads, writes logs, and starts the HTTP server.
- `sd_driver.c` / `sd_driver.h` — Platform-specific SD card and FatFS glue. Responsible for:
    - SPI initialization for the SD peripheral,
    - Mounting/unmounting the FatFS filesystem,
    - Providing simple helpers to append/read/rotate logs,
    - Exposing error/status codes for the application.
- `http_server_driver.c` / `http_server_driver.h` — Minimal HTTP server implementation that serves `index.html` and provides endpoints to list/download logs. Integrates with the project's network stack.
- `pico3_driver.c` / `pico3_driver.h` — Board-specific helpers and sensor read abstractions (reading ADCs, timestamps, or sensor drivers) used by `main.c`.
- `wifi_driver.c` / `wifi_driver.h` — Wi‑Fi helpers (Pico W) used to bring up networking for remote access and MQTT telemetry.
- `mqtt_driver.c` / `mqtt_driver.h` — Optional MQTT client wrapper to publish events or status updates to a broker.
- `timestamp_driver.c` / `timestamp_driver.h` — Timekeeping helpers for timestamping logs (RTC/timestamp handling).
- `hw_config.c` / other `*.hw_config.*` — Board and pin configuration files mapping sensors and SPI pins.
- `index.html` — Simple web dashboard served by `http_server_driver` for viewing or downloading logs.
- `lwipopts.h` — lwIP configuration used by the HTTP server and any networked features.
- `secrets.h` — Local credentials (Wi‑Fi SSID/password, MQTT broker) — keep out of public version control.

Important notes
-----------------------------
- Evaluate `sd_driver.*` and how it interfaces with FatFS — check mount/unmount logic, error handling, and file write paths.
- Review `http_server_driver.*` and `index.html` to see how logs are exposed or streamed to a client browser.
- `main.c` contains the high-level flow; check how sensor reads are batched into log writes and how the system reacts to SD failures.
- Networking components (`wifi_driver`, `mqtt_driver`) are auxiliary — the core grading focus should be on SD/FatFS integration and data integrity.

Appendix — Original installation & build notes 
---

## Installing SD Card Support (FatFs SPI Driver)

This project uses the **no-OS-FatFS-SD-SPI-RPi-Pico** library to provide SD card access over SPI.
Follow these steps to install it at the correct folder level.

---

## 1. Clone the SD card library

From your **project root directory**, run:

```sh
git clone https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico.git
```

Your folder structure should now look like this:

```
/project-root
        /Pico3
        /no-OS-FatFS-SD-SPI-RPi-Pico
```

---

## 2. No submodule setup required

This library does **not** use git submodules.
You can use it immediately after cloning.

---

## 3. Ensure your Pico project includes and links it

Your Pico project's CMakeLists.txt should contain:

```
add_subdirectory(../no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI FatFs_SPI)
target_link_libraries(pico3 FatFs_SPI)
```

---

Place the cloned library at the path expected by your `CMakeLists.txt`, or update the CMake variables to point at the library location.


