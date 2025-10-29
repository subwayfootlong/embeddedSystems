cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED PICO_SDK_PATH)
  set(PICO_SDK_PATH "C:/msys64/home/Brian/pico/pico-sdk")
endif()
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(mq7_pico C CXX ASM)
pico_sdk_init()

add_executable(mq7_read
    src/main.c
    src/MQ7Functions.c
)

target_include_directories(mq7_read PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(mq7_read
    pico_stdlib
    hardware_adc
)

pico_enable_stdio_usb(mq7_read 1)
pico_enable_stdio_uart(mq7_read 0)

# THIS line generates .uf2/.bin/.elf outputs:
pico_add_extra_outputs(mq7_read)
