cmake_minimum_required(VERSION 3.13)

project(demomq2 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(demomq2
    main.c
    mq2_driver.c
)

pico_add_extra_outputs(demomq2)

target_link_libraries(demomq2 pico_stdlib pico_time hardware_adc hardware_gpio)

# Enable USB serial
pico_enable_stdio_usb(demomq2 1)
pico_enable_stdio_uart(demomq2 0)