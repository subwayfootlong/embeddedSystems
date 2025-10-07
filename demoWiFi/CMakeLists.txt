cmake_minimum_required(VERSION 3.13)

# Import the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(demoWiFi C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize Pico SDK
pico_sdk_init()

# Add executable
add_executable(demoWiFi
    main.c
    wifi_driver.c
)

# Include lwipopts.h from current folder
target_include_directories(demoWiFi PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Link required libraries - added MQTT library
target_link_libraries(demoWiFi
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
)

# Enable USB serial
pico_enable_stdio_usb(demoWiFi 1)
pico_enable_stdio_uart(demoWiFi 0)

# Generate UF2/BIN/MAP/HEX
pico_add_extra_outputs(demoWiFi)