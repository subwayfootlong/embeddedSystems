cmake_minimum_required(VERSION 3.13)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(lab4 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(lab4
    main.c
    functions.c
)

target_link_libraries(lab4
    pico_stdlib
    hardware_pwm
    hardware_adc
)

pico_enable_stdio_usb(lab4 1)
pico_enable_stdio_uart(lab4 0)

pico_add_extra_outputs(lab4)