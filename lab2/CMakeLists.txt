cmake_minimum_required(VERSION 3.13)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project and C/C++ standards
project(lab2 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add executable (use your main source file)
add_executable(lab2
    main.c
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(lab2)

# Link to pico_stdlib (gpio, time, etc.)
target_link_libraries(lab2
    pico_stdlib
)

# Enable usb output, disable uart output (match lab3 behavior)
pico_enable_stdio_usb(lab2 1)
pico_enable_stdio_uart(lab2 0)