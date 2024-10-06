cmake_minimum_required(VERSION 3.12)
# initialize pico-sdk from submodule, this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)
project(rpi-pico C CXX ASM)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
include(configs.cmake)

add_compile_options(-Wall -O2)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()
add_subdirectory(src/)
