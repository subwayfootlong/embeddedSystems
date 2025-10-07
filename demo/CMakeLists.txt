cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project
project(demo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Add no-OS-FatFS-SD-SPI-RPi-Pico library
# Path assumes library is cloned in the labs folder
add_subdirectory(../no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

# Tell CMake where to find the executable source files
add_executable(demo 
    main.c
    functions.c
    hw_config.c
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(demo)

# Include directories
target_include_directories(demo PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI
)

# Link to libraries
target_link_libraries(demo 
    pico_stdlib
    hardware_spi
    FatFs_SPI
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(demo 1)
pico_enable_stdio_uart(demo 0)