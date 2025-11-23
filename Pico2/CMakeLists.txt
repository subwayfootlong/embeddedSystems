# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Pico2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Pico2
    main.c
    acd1100.c
    ema_filter.c
    mqtt_driver.c
    wifi_driver.c
    power_manager.c
)

target_include_directories(Pico2 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_SDK_PATH}/lib/lwip/src/include        # for lwip/apps/mqtt.h
    ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_arch/include # for pico/cyw43_arch.h
)

# Enable USB stdio; disable UART stdio
pico_enable_stdio_usb(Pico2 1)
pico_enable_stdio_uart(Pico2 0)

# pull in common dependencies
target_link_libraries(Pico2 
    pico_stdlib
    hardware_i2c
    hardware_vreg
    hardware_clocks 
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(Pico2 pico_cyw43_arch_none)
endif()

# create map/bin/hex file etc.
pico_add_extra_outputs(Pico2)

# add url via pico_set_program_url
