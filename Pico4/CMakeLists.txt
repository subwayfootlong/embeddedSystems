cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico4 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/../tflmicro
    ${CMAKE_CURRENT_BINARY_DIR}/tflmicro
)

add_executable(pico4
    main.c
    wifi_driver.c
    mqtt_driver.c
    model_data.cc
    ml_inference.cpp
)


target_include_directories(pico4 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../tflmicro/src
)


target_link_libraries(pico4
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    pico-tflmicro
)

pico_enable_stdio_usb(pico4 1)
pico_enable_stdio_uart(pico4 0)
pico_add_extra_outputs(pico4)
