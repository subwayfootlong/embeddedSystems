# ---- Project setup ----
cmake_minimum_required(VERSION 3.13)

# Project name and C standard
project(Pico1 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ---- Include SDK and Extras ----
# Import the Pico SDK
include(pico_sdk_import.cmake)

# Initialize the SDK
pico_sdk_init()

# ---- Add Executable ----
add_executable(Pico1
    main.c
    wifi_driver.c
    mqtt_driver.c
    acd1100.c
)

# ---- Include Directories ----
target_include_directories(Pico1 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_EXTRAS_PATH}/src/rp2_common/pico_sleep/include
    ${PICO_EXTRAS_PATH}/src/rp2_common/hardware_rosc/include
    ${PICO_EXTRAS_PATH}/src/rp2_common/pico_aon_timer/include
)

# ---- Link Libraries ----
target_link_libraries(Pico1
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background  
    hardware_vreg
    hardware_clocks                              
    hardware_gpio
    hardware_rtc
    hardware_i2c
    hardware_adc
    pico_lwip_mqtt
)

# ---- Enable standard outputs ----
pico_enable_stdio_usb(Pico1 1)
pico_enable_stdio_uart(Pico1 0)

# ---- Create UF2, bin, and hex files ----
pico_add_extra_outputs(Pico1)
