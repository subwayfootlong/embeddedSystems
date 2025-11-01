cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED PICO_SDK_PATH)
  set(PICO_SDK_PATH "C:/msys64/home/Brian/pico/pico-sdk")
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(mq135_pico C CXX ASM)

pico_sdk_init()

add_executable(mq135_read
    src/main.c
    src/mq135_driver.c
)

target_include_directories(mq135_read PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(mq135_read
    pico_stdlib
    hardware_adc
)

pico_enable_stdio_usb(mq135_read 1)
pico_enable_stdio_uart(mq135_read 0)

pico_add_extra_outputs(mq135_read)