cmake_minimum_required(VERSION 3.13)

# Import the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(demomqtt C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize Pico SDK
pico_sdk_init()

# Add executable
add_executable(demomqtt
    main.c
    wifi_driver.c
    mqtt_driver.c
)

# Include lwipopts.h from current folder
target_include_directories(demomqtt PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Link required libraries
target_link_libraries(demomqtt
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
)

# Enable USB serial
pico_enable_stdio_usb(demomqtt 1)
pico_enable_stdio_uart(demomqtt 0)

# Generate UF2/BIN/MAP/HEX
pico_add_extra_outputs(demomqtt)