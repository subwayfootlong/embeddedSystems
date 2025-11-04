cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(Pico3 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Remove this line: add_subdirectory(../no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

add_executable(Pico3
    main.c
    pico3_driver.c
    wifi_driver.c
    mqtt_driver.c
    sd_driver.c
    hw_config.c
    timestamp_driver.c
    http_server_driver.c
)

pico_add_extra_outputs(Pico3)

target_include_directories(Pico3 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI
)

target_link_libraries(Pico3
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    pico_lwip_http
    hardware_spi
    FatFs_SPI
)

pico_enable_stdio_usb(Pico3 1)
pico_enable_stdio_uart(Pico3 0)