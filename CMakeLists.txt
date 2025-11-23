cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Force Pico W so Wi‑Fi/LwIP libraries and headers are available across subprojects.
set(PICO_BOARD pico_w CACHE STRING "Board type" FORCE)

project(PicoProjects C CXX ASM)

# Initialize Pico SDK
pico_sdk_init()

# Add FatFs_SPI library only once
if(NOT TARGET FatFs_SPI)
    add_subdirectory(no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)
endif()

# Add individual projects
if(NOT PICO_NO_HARDWARE)
    add_subdirectory(demoSD)
    add_subdirectory(demoWiFi)
    add_subdirectory(demoMQTT)
    add_subdirectory(Pico2)
    add_subdirectory(Pico3)
    add_subdirectory(Pico4)
endif()
