cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(PicoProjects C CXX ASM)

# Initialize Pico SDK
pico_sdk_init()

# Add FatFs_SPI library only once
if(NOT TARGET FatFs_SPI)
    add_subdirectory(no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)
endif()

# Add individual projects
if(NOT PICO_NO_HARDWARE)
    add_subdirectory(lab1)
    add_subdirectory(lab3)
    add_subdirectory(lab4)
    add_subdirectory(lab6)
endif()