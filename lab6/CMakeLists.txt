cmake_minimum_required(VERSION 3.13)

project(lab6_pid C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# If PICO_SDK_PATH wasn't provided by a top-level CMake, import the SDK here.
# This makes the file usable both as a subdirectory of the examples repo
# (top-level already imported the SDK) and standalone.
if (NOT DEFINED PICO_SDK_PATH)
    include(pico_sdk_import.cmake)
endif()

pico_sdk_init()

add_executable(lab6
    main.c
    functions.c
)

target_include_directories(lab6 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link Pico stdlib (provides retargeted printf, sleep, etc.)
target_link_libraries(lab6 PRIVATE pico_stdlib)

# Create UF2/HEX/ELF outputs when using the board
pico_add_extra_outputs(lab6)