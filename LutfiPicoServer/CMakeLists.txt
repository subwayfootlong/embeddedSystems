cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(LutfiPicoServer C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Remove this line: add_subdirectory(../no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

add_executable(LutfiPicoServer 
    main.c
    server_driver.c
    wifi_driver.c
    mqtt_driver.c
    sd_driver.c
    hw_config.c
    timestamp_driver.c
)

pico_add_extra_outputs(LutfiPicoServer)

target_include_directories(LutfiPicoServer PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI
)

target_link_libraries(LutfiPicoServer
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    hardware_spi
    FatFs_SPI
)

pico_enable_stdio_usb(LutfiPicoServer 1)
pico_enable_stdio_uart(LutfiPicoServer 0)